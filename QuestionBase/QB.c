#include "QB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>

// Not DHCP, i doubt thats a requirement?
// based on this https://www.sanfoundry.com/c-program-get-ip-address/
// https://www.includehelp.com/c-programs/get-ip-address-in-linux.aspx
void get_local_ip(char *ip, int size) {
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    char host[NI_MAXHOST];
    int found = 0;

    // Get list of network interfaces and their associated addresses
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    // Iterate over list of network interfaces
    for (ifa = ifaddr; ifa != NULL && !found; ifa = ifa->ifa_next) {
        // Retriee IPV4 addredd
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET) {
            // Make sure that it doesnt use local host
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            // inet_ntoa convert and return ip address in array format
            if (strcmp(inet_ntoa(sa->sin_addr), "127.0.0.1") != 0) {
               // Host name ip and address to buffer
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
                    strncpy(ip, host, size); // store iP, host and size
                    found = 1; // On successful find
                }
            }
        }
    }

    freeifaddrs(ifaddr); // Free ifaddr memory
    // If it fails to find the computer's IP address
    if (!found) {
        fprintf(stderr, "Unable to find local IP address\n");
        exit(EXIT_FAILURE);
    }
}

void create_socket(int *sockfd) {
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void bind_socket(int sockfd) {
    struct sockaddr_in server_addr;
    char ip[NI_MAXHOST];
    int port = 9001;

    // Get local IP address
    get_local_ip(ip, sizeof(ip));
    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    // Bind the socket to a specific address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Server running on %s:%d\n", ip, port);
    }
}

void listen_for_connections(int sockfd) {
    if (listen(sockfd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

void handle_connection(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SIZE];
    int connfd;

    if ((connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    int len = recv(connfd, buf, BUF_SIZE, 0);
    if (len < 0) {
        perror("recv failed");
        exit(EXIT_FAILURE);
    }

    buf[len] = '\0';
    printf("Received message: %s\n", buf);

    // Send acknowledgement back to sender
    char ack_msg[BUF_SIZE];
    sprintf(ack_msg, "Acknowledgement for %s", buf);
    if (send(connfd, ack_msg, strlen(ack_msg), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    close(connfd); // Close the connection
}

void close_connection(int connfd) {
    close(connfd);
}
