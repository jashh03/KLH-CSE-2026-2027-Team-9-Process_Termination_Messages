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

    // Predefined client information
    request.client_id = 1;
    strcpy(request.message, "Hello Server");

    char client_fifo[100];
    sprintf(client_fifo, "/tmp/client_%d_fifo", request.client_id);

    // Create client FIFO
    mkfifo(client_fifo, 0666);

    // Connect to server FIFO
    int server_fd = open(SERVER_FIFO, O_WRONLY);

    if (server_fd == -1) {
        perror("Error: Server is not running");
        return 1;
    }

    // Send request
    write(server_fd, &request, sizeof(Request));

    printf("Client %d sent: %s\n",
           request.client_id,
           request.message);

    close(server_fd);

    // Wait for server response
    int client_fd = open(client_fifo, O_RDONLY);

    if (client_fd == -1) {
        perror("Client FIFO error");
        return 1;
    }

    char response[BUFFER_SIZE];

    read(client_fd, response, BUFFER_SIZE);

    printf("Server Response: %s\n", response);

    close(client_fd);

    unlink(client_fifo);

    return 0;
}
