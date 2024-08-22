#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define PORT	1111
#define BACKLOG 16
#define BUF_SIZE 2048

static char buffer[BUF_SIZE];

int main() {
    int sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock_fd < 0){
        perror("socket"); exit(EXIT_FAILURE);
    }
    struct sockaddr_in sock_addr = {0};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0){
        perror("bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    if(listen(sock_fd, BACKLOG) < 0){
        perror("listen");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        int conn_fd = accept(sock_fd, 0, 0);
        if(conn_fd < 0){
            perror("accept");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        int n = recv(conn_fd, buffer, BUF_SIZE, 0);
        if(n < 0){
            perror("recv");
            close(conn_fd);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        if(send(conn_fd, buffer, n, 0) < 0){
            perror("send");
            close(conn_fd);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        shutdown(conn_fd, SHUT_RDWR);
        close(conn_fd);
    }
    return EXIT_SUCCESS;
}
