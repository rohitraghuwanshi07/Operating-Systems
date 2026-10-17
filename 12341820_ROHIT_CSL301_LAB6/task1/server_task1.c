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

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    printf("========================================\n");
    printf("     TCP UNIT CONVERTER SERVER\n");
    printf("========================================\n");
    printf("Server started on port %d\n", PORT);
    printf("Waiting for clients...\n");

    while (1) {
        struct sockaddr_in client = {0};
        socklen_t len = sizeof(client);
        int c = accept(server_fd, (struct sockaddr *)&client, &len);
        if (c < 0) { perror("accept"); continue; }

        printf("Client connected: %s\n", inet_ntoa(client.sin_addr));

        while (1) {
            char req[BUFFER_SIZE] = {0};
            ssize_t n = recv(c, req, sizeof(req) - 1, 0);
            if (n <= 0) break;
            req[n] = '\0';
            req[strcspn(req, "\r\n")] = '\0';

            printf("Request: %s\n", req);

            char response[BUFFER_SIZE];
            memset(response, 0, sizeof(response));

            if (strcmp(req, "EXIT") == 0) {
                snprintf(response, sizeof(response), "Session terminated. Goodbye!");
                send_all(c, response, strlen(response) + 1);
                printf("Client session ended.\n");
                break;
            }

            char op[64];
            double value;
            if (sscanf(req, "%63s %lf", op, &value) != 2) {
                snprintf(response, sizeof(response),
                         "ERROR: Invalid request format.");
            } else if (strcmp(op, "CM_TO_M") == 0) {
                snprintf(response, sizeof(response),
                         "%.2f cm = %.2f m", value, value / 100.0);
            } else if (strcmp(op, "M_TO_KM") == 0) {
                snprintf(response, sizeof(response),
                         "%.2f m = %.2f km", value, value / 1000.0);
            } else if (strcmp(op, "C_TO_F") == 0) {
                snprintf(response, sizeof(response),
                         "%.2f C = %.2f F", value, (value * 9.0 / 5.0) + 32.0);
            } else if (strcmp(op, "KG_TO_G") == 0) {
                snprintf(response, sizeof(response),
                         "%.2f kg = %.2f g", value, value * 1000.0);
            } else {
                snprintf(response, sizeof(response),
                         "ERROR: Unsupported conversion '%s'.", op);
            }

            printf("Response: %s\n", response);
            send_all(c, response, strlen(response) + 1);
        }

        close(c);
        printf("Waiting for new clients...\n");
    }

    close(server_fd);
    return 0;
}
