#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>         
#include <netinet/in.h>         
#include <arpa/inet.h>        
#include <signal.h>
#include <sys/wait.h>  
#include <fcntl.h> 
#include <sys/stat.h> 
#include <errno.h>


// all requests

void request(int input) {

    char buffer[1024];

    ssize_t inputBytes = read(input, buffer, sizeof(buffer) - 1);

    if (inputBytes <= 0) {
        printf("HTTP/1.0 400 Bad Request");
        return;
    }
    buffer[inputBytes] = '\0'; 

    char method[5], out[1024], version[9];

    int number = sscanf(buffer, "%s %s %s", method, out, version);

    if (number < 3) {
        printf( "HTTP/1.0 400 Bad Request");
        return;
    }


    if (strncmp(method, "GET", 3) != 0 && strncmp(method, "HEAD", 4) != 0) {

        printf("HTTP/1.0 501 Not Implemented");
        return;
    }

    if (strcmp(out, "/") == 0 || strcmp(out, "/index.html") == 0) {
        strcpy(out, "index.html");

    } else {
        printf("HTTP/1.0 404 Not Found");
        return;
    }

    // open html

    int file_fd = open(out, O_RDONLY);

    struct stat fileStats;

    if (file_fd >= 0 && fstat(file_fd, &fileStats) >= 0) {

        char header[256];

        write(input, header, strlen(header));

        if (strncmp(method, "GET", 3) == 0) 
        { 
            while ((inputBytes = read(file_fd, buffer, sizeof(buffer))) > 0) 
            {
                write(input, buffer, inputBytes);
            }
        }

        close(file_fd);

    } else 
    {
        printf("HTTP/1.0 500 Internal Server Error");
    }
}

void run(int fd) {
    struct sockaddr_in client;
    socklen_t clientLength;

    while (1) {
        clientLength = sizeof(client);
        int input = accept(fd, (struct sockaddr *) &client, &clientLength);
        if (input < 0) {
            perror("ERROR");
            continue;
        }

        printf("Linked\n");
        request(input);
        printf("Closed\n");
        close(input); 
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR");
        exit(1);
    }

    int port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCKS, 0);

    if (sock < 0) {
        perror("ERROR");
        exit(1);
    }

    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {

        perror("ERROR");
        close(sock);
        exit(1);
    }

    listen(sock, 5); 

    printf("Server: %d\n", port);

    run(sock);

    close(sock);
    return 0;
}
