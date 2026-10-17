#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

static ssize_t send_all(int s, const char *buf, size_t n) {
    size_t sent = 0;
    while (sent < n) {
        ssize_t r = send(s, buf + sent, n - sent, 0);
        if (r <= 0) return -1;
        sent += (size_t)r;
    }
    return (ssize_t)sent;
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
        printf("Invalid server IP address.\n");
        close(sock);
        return 1;
    }

    printf("========================================\n");
    printf("       TCP UNIT CONVERTER CLIENT\n");
    printf("========================================\n");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    printf("Connected to server %s:%d\n", argv[1], PORT);
    printf("Supported conversions:\n");
    printf("CM_TO_M <value>\nM_TO_KM <value>\nC_TO_F <value>\nKG_TO_G <value>\nEXIT\n");

    while (1) {
        char req[BUFFER_SIZE];
        printf("\nEnter conversion request: ");
        fflush(stdout);

        if (!fgets(req, sizeof(req), stdin)) break;
        req[strcspn(req, "\r\n")] = '\0';

        if (send_all(sock, req, strlen(req) + 1) < 0) {
            printf("Connection error.\n");
            break;
        }

        char response[BUFFER_SIZE] = {0};
        size_t got = 0;
        while (got < sizeof(response) - 1) {
            ssize_t n = recv(sock, response + got, 1, 0);
            if (n <= 0) { got = 0; break; }
            if (response[got++] == '\0') break;
        }
        if (got == 0) break;

        printf("Server response: %s\n", response);
        if (strcmp(req, "EXIT") == 0) break;
    }

    close(sock);
    printf("Connection closed.\n");
    return 0;
}
