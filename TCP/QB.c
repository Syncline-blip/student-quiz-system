#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define PORT 9002

struct message {
    int type;      // Type of message (e.g. 0 = login, 1 = logout, 2 = send message)
    int length;    // Length of payload in bytes
    char payload[1024];  // Payload data
};

int calculate_checksum(char* buf, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

int main(int argc, char* argv[]) {
    int sockfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SIZE];
    struct message msg;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT); // bind to port 9002

    // Bind the socket to a specific address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Binded to port %d\n", PORT); // 9001 doesnt work wtf?
    }

    /* ---------- By this point the program is listening on the port (PORT value) --------- */

    // Program will serve forever - until it's closed.
    while (1) {
        // Listen for incoming connections
        if (listen(sockfd, 5) < 0) {
            perror("listen failed");
            exit(EXIT_FAILURE);
        }

        // Accept incoming connection
        connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Use read to read message received
        int len = read(connfd, &msg, sizeof(msg));
        if (len < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        /* --------- Message has been received now so now looking into the message type --------- */

        // Verify the message checksum
        int expected_checksum = calculate_checksum((char*)&msg, sizeof(msg) - sizeof(int));
        if (expected_checksum != msg.type) {
            printf("Invalid message checksum\n");
            send(connfd, "CHECKSUM FAILED", strlen("CHECKSUM FAILED"), 0);
            continue;
        }

        // Print the message payload
        printf("Received message of type %d, length %d: %s\n", msg.type, msg.length, msg.payload);

        // Basic ACK, needs to acknowledge received and failed messages
        char ack_msg[BUF_SIZE];
        sprintf(ack_msg, "ACK for: '%s'", msg.payload);
        if (send(connfd, ack_msg, strlen(ack_msg), 0) < 0) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        // Close socket connection
        close(connfd);
    }
    close(sockfd);
    return 0;
}
