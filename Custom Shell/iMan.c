#include "iMan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 8192
#define PORT 80

void fetch_man_page(const char *command, ManPage *man_page) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[1024];
    char response[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    size_t total_bytes = 0;
    char *body_start;

    // Initialize ManPage struct
    man_page->length = 0;
    memset(man_page->content, 0, sizeof(man_page->content));

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Resolve the host
    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Prepare and send the HTTP GET request
    snprintf(request, sizeof(request),
             "GET /?topic=%s&section=all HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n", 
             command);
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("write");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive the response
    while ((bytes_received = read(sockfd, response + total_bytes, sizeof(response) - total_bytes - 1)) > 0) {
        total_bytes += bytes_received;
    }

    if (bytes_received < 0) {
        perror("read");
    }

    // Find the end of the headers
    response[total_bytes] = '\0'; // Null-terminate the response
    body_start = strstr(response, "\n\n");
    if (body_start != NULL) {
        // Move body_start to the start of the body
        body_start += 4; // Skip past "\r\n\r\n"
        size_t body_length = total_bytes - (body_start - response);
        if (body_length > sizeof(man_page->content) - 1) {
            body_length = sizeof(man_page->content) - 1;
        }
        memcpy(man_page->content, body_start, body_length);
        man_page->content[body_length] = '\0'; // Null-terminate the content
        man_page->length = body_length;
    } else {
        fprintf(stderr, "Error: Could not find the end of the headers in the response\n");
    }

    // Close the socket
    close(sockfd);
}
