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
static int recv_cstr(int s, char *buf, size_t cap) {
    size_t i = 0;
    while (i + 1 < cap) {
        char ch;
        ssize_t r = recv(s, &ch, 1, 0);
        if (r <= 0) return -1;
        if (ch == '\0') { buf[i] = '\0'; return 0; }
        buf[i++] = ch;
    }
    buf[cap - 1] = '\0';
    return 0;
}

int main(void) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server); return 1;
    }
    if (listen(server, 5) < 0) {
        perror("listen"); close(server); return 1;
    }

    printf("========================================\n");
    printf("       TCP FILE TRANSFER SERVER\n");
    printf("========================================\n");
    printf("Server started on port %d\nWaiting for clients...\n", PORT);

    while (1) {
        struct sockaddr_in client = {0};
        socklen_t len = sizeof(client);
        int c = accept(server, (struct sockaddr *)&client, &len);
        if (c < 0) continue;

        printf("Client connected: %s\n", inet_ntoa(client.sin_addr));

        while (1) {
            char cmd[32] = {0};
            if (recv_cstr(c, cmd, sizeof(cmd)) < 0) break;
            printf("Command received: %s\n", cmd);

            if (strcmp(cmd, "EXIT") == 0) {
                printf("Client requested exit.\n");
                break;
            }

            char filename[NAME_SIZE] = {0};
            if (recv_cstr(c, filename, sizeof(filename)) < 0) break;

            if (strcmp(cmd, "UPLOAD") == 0) {
                uint64_t size;
                if (recv_all(c, &size, sizeof(size)) < 0) break;

                printf("Upload request: %s (%llu bytes)\n",
                       filename, (unsigned long long)size);

                FILE *fp = fopen(filename, "wb");
                if (!fp) {
                    char status[] = "ERROR";
                    send_all(c, status, sizeof(status));
                    continue;
                }

                uint64_t left = size;
                char buffer[BUFFER_SIZE];
                int ok = 1;
                while (left) {
                    size_t want = left > BUFFER_SIZE ? BUFFER_SIZE : (size_t)left;
                    ssize_t n = recv(c, buffer, want, 0);
                    if (n <= 0) { ok = 0; break; }
                    if (fwrite(buffer, 1, (size_t)n, fp) != (size_t)n) { ok = 0; break; }
                    left -= (uint64_t)n;
                }
                fclose(fp);

                if (ok) {
                    const char status[] = "UPLOAD_SUCCESS";
                    send_all(c, status, sizeof(status));
                    printf("File uploaded successfully: %s\n", filename);
                } else remove(filename);
            } else if (strcmp(cmd, "DOWNLOAD") == 0) {
                FILE *fp = fopen(filename, "rb");
                if (!fp) {
                    uint64_t size = UINT64_MAX;
                    send_all(c, &size, sizeof(size));
                    printf("File not found: %s\n", filename);
                    continue;
                }

                fseek(fp, 0, SEEK_END);
                long s = ftell(fp);
                rewind(fp);
                uint64_t size = (uint64_t)s;
                send_all(c, &size, sizeof(size));

                char buffer[BUFFER_SIZE];
                size_t n;
                while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
                    if (send_all(c, buffer, n) < 0) break;

                fclose(fp);
                printf("File sent successfully: %s (%llu bytes)\n",
                       filename, (unsigned long long)size);
            } else {
                printf("Invalid command received.\n");
            }
        }

        close(c);
        printf("Client session ended.\nWaiting for new clients...\n");
    }

    close(server);
    return 0;
}
