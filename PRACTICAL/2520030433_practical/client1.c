#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/calculator.sock"

int main() {
    int sock;
    struct sockaddr_un addr;
    char buffer[100];

    // Create socket
    sock = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock == -1) {
        perror("socket");
        return 1;
    }

    // Specify server socket
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Error: Cannot connect to server");
        close(sock);
        return 1;
    }

    // Send message
    strcpy(buffer, "Hello Server");
    write(sock, buffer, strlen(buffer) + 1);

    printf("Client sent: %s\n", buffer);

    // Receive response
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));

    printf("Server Response: %s\n", buffer);

    close(sock);

    return 0;
}
