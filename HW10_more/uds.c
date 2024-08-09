#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#define NAME "echo.socket"


int main()
{
    int sock, msgsock, rval;
    struct sockaddr_un server;
    struct stat st;
    char buf[1024];
    //const char * filename;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, NAME);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) {
        perror("binding stream socket");
        exit(1);
    }
    printf("Socket has name %s\n", server.sun_path);
    listen(sock, 2);
    for (;;) {
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
        {
            perror("accept");
            break;
        }
        else {
            bzero(buf, sizeof(buf));

            int ret = stat("/home/user/Downloads/GoodbyeDPI-0.2.2.zip", &st);
            if (ret < 0) {
                sprintf(buf, "Can`t read file size. %s\n", strerror(errno)); //#include <string.h>
            } 
            else {
                off_t size = st.st_size;
                sprintf(buf, "Size of file is %ld bytes\n", size);
                }
            send(msgsock, buf, strlen(buf), 0);
        } //while (rval > 0);
        close(msgsock);
    }
    close(sock);
    unlink(NAME);
}