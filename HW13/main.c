#define _XOPEN_SOURCE 700
#include <bsd/string.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define CONNBREAK "\033[m"
#define BUFFER_SZ 4000
#define SERVICE "telehack.com"
#define TELNETPORT "23"

void recieve_text_data(int sck_fd, char *text) {
  char inp_buf[BUFFER_SZ];
  int inbytes;
  // index of text
  int k = 0;
  do {
    inbytes = recv(sck_fd, inp_buf, BUFFER_SZ - 1, 0);
    char IAC = (char)255; // IAC Telnet byte consists of all ones
    for (int i = 0; i < inbytes; i++) {
      if (inp_buf[i] == IAC) {
        // command sequence follows
        i++;
        switch (inp_buf[i]) {
        case ((char)251):
          // WILL as input
          i++;
          break;
        case ((char)252):
          // WON'T as input
          break;
        case ((char)253):
          // DO as input
          i++;
          break;
        case ((char)254):
          // DON'T as input
          break;
        default:
          // not a option negotiation, a genereal TELNET command like IP, AO, etc.
          break;
        }
      } else {
        // is data
        text[k] = inp_buf[i];
        k++;
        // printf("%c", inp_buf[i]);
      }
    }
    // serch last bytes as "\r\n.\0 for break"
    if (inp_buf[inbytes - 1] == '.' && inp_buf[inbytes - 2] == '\n' &&
        inp_buf[inbytes - 3] == '\r') {
      text[k - 1] = '\n';
      text[k] = '\0';
      break;
    }
  } while (inbytes > 0);
}

char *concat(int count, ...) {
  va_list ap;
  // Find required length to store merged string
  int len = 1; // room for NULL
  va_start(ap, count);
  for (int i = 0; i < count; i++)
    len += strlen(va_arg(ap, char *));
  va_end(ap);

  // Allocate memory to concat strings
  char *merged = (char *)calloc(sizeof(char), len);
  int null_pos = 0;

  // Actually concatenate strings
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    char *s = va_arg(ap, char *);
    size_t s_length = strlen(s);
    strlcpy(merged + null_pos, s, s_length + 1);
    null_pos += strlen(s);
  }
  va_end(ap);

  return merged;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf(
        "Use main <font_name> <text_to_convert>\n\
Examples: 1) ./main zc Hello!\n\t  2) ./main script 12655\n");
    return 0;
  }
  char text[BUFFER_SZ] = {0};
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  struct addrinfo *results;
  if (getaddrinfo(SERVICE, TELNETPORT, &hints, &results) != 0) {
    printf("Error in getaddrinfo()\n");
    return EXIT_FAILURE;
  }

  struct addrinfo *addr_okay;
  int succeeded = 0;
  int sck_fd = socket(AF_INET, SOCK_STREAM, 0);
  for (addr_okay = results; addr_okay != NULL; addr_okay = addr_okay->ai_next) {
    if (connect(sck_fd, addr_okay->ai_addr, addr_okay->ai_addrlen) >= 0) {
      succeeded = 1;
      // printf("connected successfully!\n");
      break;
    }
  }
  if (!succeeded) {
    printf("error, coudln't connect() to any of possible addresses\n");
    return EXIT_SUCCESS;
  }
  //printf("Connected!\n");
  freeaddrinfo(results);
  //Начальное приглашение
  recieve_text_data(sck_fd, text);
  if (strcmp(text, CONNBREAK) == 0){
      printf("Connection closed by remote host.\nProbaly your IP banned on server.\n");
      return EXIT_FAILURE;
  }
  char * senddata = concat(6, "\xFF\xFD\xFF", " figlet /", argv[1], " ", argv[2],
                    "\r\n\0"); // figlet /sblood HELLO"
  send(sck_fd, senddata, strlen(senddata), 0);
  recieve_text_data(sck_fd, text);
  printf("%s", &text[strlen(senddata) - 4]); // 4 - "\xFF\xFD\xFF "
  free(senddata);
  close(sck_fd);
  return 0;
}