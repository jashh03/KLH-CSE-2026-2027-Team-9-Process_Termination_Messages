#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define BUFFER_SIZE 256

typedef struct {
    int client_id;
    char message[BUFFER_SIZE];
} Request;

int main() {
    Request request;
    int fd;
    char response[BUFFER_SIZE];

    // Create server FIFO
    mkfifo(SERVER_FIFO, 0666);

    printf("Server started...\n");
    printf("Waiting for client messages...\n");

    while (1) {

        // Open server FIFO for reading
        fd = open(SERVER_FIFO, O_RDONLY);

        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Read request
        while (read(fd, &request, sizeof(Request)) > 0) {

            printf("\nReceived from Client %d: %s",
                   request.client_id, request.message);

            // Process message
            snprintf(response, BUFFER_SIZE,
                     "Server response to Client %d: Message received successfully!",
                     request.client_id);

            // Create client FIFO name
            char client_fifo[100];
            sprintf(client_fifo,
                    "/tmp/client_%d_fifo",
                    request.client_id);

            // Open client's FIFO
            int client_fd = open(client_fifo, O_WRONLY);

            if (client_fd != -1) {
                write(client_fd, response, strlen(response) + 1);
                close(client_fd);
            }

            printf("Response sent to Client %d\n",
                   request.client_id);
        }

        close(fd);
    }

    unlink(SERVER_FIFO);

    return 0;
}
