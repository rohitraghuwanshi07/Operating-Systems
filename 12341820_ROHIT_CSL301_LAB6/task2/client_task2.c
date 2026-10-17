#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define NAME_SIZE 256

static int send_all(int s, const void *buf, size_t n) {
    const char *p = buf;
    while (n) {
        ssize_t r = send(s, p, n, 0);
        if (r <= 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}
static int recv_all(int s, void *buf, size_t n) {
    char *p = buf;
    while (n) {
        ssize_t r = recv(s, p, n, 0);
        if (r <= 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}
static int send_cstr(int s, const char *str) {
    return send_all(s, str, strlen(str) + 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server-ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0) {
        printf("Invalid server IP.\n"); close(sock); return 1;
    }

    printf("========================================\n");
    printf("        TCP FILE TRANSFER CLIENT\n");
    printf("========================================\n");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    printf("Connected to server %s:%d\n", argv[1], PORT);

    while (1) {
        printf("====================================\n");
        printf("       FILE TRANSFER SYSTEM\n");
        printf("====================================\n");
        printf("1. Upload File\n2. Download File\n3. Exit\n");
        printf("====================================\n");
        printf("Enter choice: ");

        char choice[16];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        int ch = atoi(choice);

        if (ch == 1) {
            char filename[NAME_SIZE];
            printf("Enter filename to upload: ");
            if (!fgets(filename, sizeof(filename), stdin)) break;
            filename[strcspn(filename, "\r\n")] = '\0';

            FILE *fp = fopen(filename, "rb");
            if (!fp) { perror("fopen"); continue; }

            fseek(fp, 0, SEEK_END);
            long s = ftell(fp);
            rewind(fp);
            uint64_t size = (uint64_t)s;

            send_cstr(sock, "UPLOAD");
            send_cstr(sock, filename);
            send_all(sock, &size, sizeof(size));

            char buffer[BUFFER_SIZE];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
                if (send_all(sock, buffer, n) < 0) break;
            fclose(fp);

            char status[32] = {0};
            size_t i = 0;
            while (i + 1 < sizeof(status)) {
                if (recv(sock, &status[i], 1, 0) <= 0) break;
                if (status[i++] == '\0') break;
            }

            if (strcmp(status, "UPLOAD_SUCCESS") == 0) {
                printf("File uploaded successfully.\nFilename: %s\nSize: %llu bytes\n",
                       filename, (unsigned long long)size);
            } else {
                printf("ERROR: Upload failed.\n");
            }
        } else if (ch == 2) {
            char filename[NAME_SIZE];
            printf("Enter filename to download: ");
            if (!fgets(filename, sizeof(filename), stdin)) break;
            filename[strcspn(filename, "\r\n")] = '\0';

            send_cstr(sock, "DOWNLOAD");
            send_cstr(sock, filename);

            uint64_t size;
            if (recv_all(sock, &size, sizeof(size)) < 0) break;

            if (size == UINT64_MAX) {
                printf("ERROR: File not found on server: %s\n", filename);
                continue;
            }

            FILE *fp = fopen(filename, "wb");
            if (!fp) {
                printf("ERROR: Cannot create local file.\n");
                continue;
            }

            uint64_t left = size;
            char buffer[BUFFER_SIZE];
            int ok = 1;
            while (left) {
                size_t want = left > BUFFER_SIZE ? BUFFER_SIZE : (size_t)left;
                if (recv_all(sock, buffer, want) < 0) { ok = 0; break; }
                if (fwrite(buffer, 1, want, fp) != want) { ok = 0; break; }
                left -= want;
            }
            fclose(fp);

            if (ok)
                printf("File downloaded successfully.\nFilename: %s\nSize: %llu bytes\n",
                       filename, (unsigned long long)size);
        } else if (ch == 3) {
            printf("Connection closing...\n");
            send_cstr(sock, "EXIT");
            break;
        } else {
            printf("Invalid choice. Please select 1, 2, or 3.\n");
        }
    }

    close(sock);
    printf("Connection closed.\n");
    return 0;
}
