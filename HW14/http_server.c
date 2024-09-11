#define _GNU_SOURCE

#include <arpa/inet.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define THREADS_NUM 12
#define MAX_OPEN_FILES_DESCRIPTORS 10000
#define MAX_CONNECTIONS 10000
#define BUFFER_SIZE 4096
#define MAX_EVENTS (MAX_CONNECTIONS) + 2 //+1 socket_fd and

#define MAX_URL 4096
#define MAX_METHOD 8
#define MAX_VERSION 16

#define HEADER_SERVERNAME "Server: epoll-webserver\r\n"
#define HEADER_DATE "Date: %s\r\n"
#define REPLY_200                                                              \
  "HTTP/1.0 200 OK\r\n" HEADER_SERVERNAME HEADER_DATE                          \
  "Content-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n"
#define REPLY_400                                                              \
  "HTTP/1.0 400 Bad Request\r\n" HEADER_SERVERNAME HEADER_DATE "\r\n"
#define REPLY_403                                                              \
  "HTTP/1.0 403 Forbidden\r\n" HEADER_SERVERNAME HEADER_DATE "\r\n"
#define REPLY_404                                                              \
  "HTTP/1.0 404 Not Found\r\n" HEADER_SERVERNAME HEADER_DATE "\r\n"
#define REPLY_405                                                              \
  "HTTP/1.0 405 Method Not Allowed\r\nAllow: GET\r\n" HEADER_SERVERNAME        \
      HEADER_DATE "\r\n"
#define REPLY_413                                                              \
  "HTTP/1.0 413 Payload Too Large\r\n" HEADER_SERVERNAME HEADER_DATE "\r\n"

//Массив буферов
char *buffers;
ssize_t *buffer_lengths;

//Тут будем хранить массив длин заголовков сервера при ответе
ssize_t *resp_header_length;
//Тут будем хранить массив дескритора файла при отправке
int *file_fds;
//Тут будем хранить массив размера файла при отправке
ssize_t *file_length;

int epoll_fd;
static char *pubdir;

typedef struct {
  int clientfd; // client file descriptor
  char uri[MAX_URL];
  char method[MAX_METHOD];
  char version[MAX_VERSION];
} request_t;

typedef struct {
  in_addr_t host;
  uint16_t port;
} thread_args;

typedef enum {
  STATUS_ACCEPT = 0,
  STATUS_READ,
  STATUS_WRITE,
  STATUS_FINISHED,
} event_status_t;

typedef struct {
  int fd;
  off_t size;
} file_t;

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

uint64_t pack_event_to_clientfd(int client_fd, event_status_t event_type) {
  return (uint64_t)(event_type) << 32 | client_fd;
}

int unpack_client_fd(uint64_t request_id) {
  // UNIT_MAX = 0x00000000FFFFFFFF
  return request_id & UINT_MAX;
}

event_status_t unpack_client_status(uint64_t request_id) {
  // ULLONG_MAX - UINT_MAX = 0xFFFFFFFF00000000
  return (request_id & (ULLONG_MAX - UINT_MAX)) >> 32;
}

char *client_buffer(int client_fd) { return &buffers[client_fd * BUFFER_SIZE]; }

void setup_rlimit_nofile(long long limit) {
  struct rlimit file_limit;
  int res = getrlimit(RLIMIT_NOFILE, &file_limit);
  if (res != 0)
    perror("getrlimit");
  file_limit.rlim_cur = limit;
  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0)
    perror("setrlimit");
  printf("Limit of file descriptors is: %ld\n", file_limit.rlim_cur);
}

void die(const char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}

void set_listener_socket_opts(int sockfd) {
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    die("setsockopt(SO_RCVTIMEO) failed");

  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0)
    die("setsockopt(SO_SNDTIMEO) failed\n");

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    die("setsockopt(SO_REUSEADDR) failed");

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    die("setsockopt(SO_REUSEPORT) failed");
}

/* Add fd+event_type to epoll */
void add_efd_to_epoll(int epollfd, int fd, event_status_t status,
                      uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.ptr = (void *)pack_event_to_clientfd(fd, status);
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    die("epoll_ctl(edf, EPOLL_CTL_ADD, fd) failed");
  }
}

void reload_efd_in_epoll(int epollfd, int fd, event_status_t status,
                         uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.ptr = (void *)pack_event_to_clientfd(fd, status);
  if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
    printf("client_fd %d\n", fd);
    die("epoll_ctl(edf, EPOLL_CTL_MOD, fd) failed");
  }
}

void del_fd_from_epoll(int efd, int fd) {
  if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) == -1) {
    perror("epoll_ctl(edf, EPOLL_CTL_DEL, fd) failed");
    exit(EXIT_FAILURE);
  }
}

file_t *get_file(const char *path) {
  // char * mfile;
  struct stat st;
  file_t *file;
  int fd = -1;
  file = malloc(sizeof(file_t));
  if (file == NULL) {
    perror("malloc for file_t");
    return NULL;
  }
  if (stat(path, &st) < 0)
    return NULL;
  if (!(st.st_mode & S_IFREG))
    return NULL;
  if ((fd = open(path, O_RDONLY, 0)) == -1) {
    perror("Error while open file");
    return NULL;
  }
  file->fd = fd;
  file->size = st.st_size;
  return file;
}

int handle_request(int client_fd, ssize_t rbytes) {
  // modify buffer_lengths[client_fd] first
  ssize_t length = (buffer_lengths[client_fd] += rbytes);
  int header_bytes;

  char date[32];
  time_t t = time(NULL);
  struct tm *tm = gmtime(&t);
  strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", tm);

  if (length == BUFFER_SIZE) {
    header_bytes =
        snprintf(client_buffer(client_fd), BUFFER_SIZE, REPLY_413, date);
    printf("To large reqest payload with client_fd %d", client_fd);
    goto exit;
  }
  request_t req;
  *(client_buffer(client_fd) + length) = '\0';
  if (strcmp((client_buffer(client_fd) + length - 4), "\r\n\r\n") == 0) {

    // printf("recv finished\n");
    if (sscanf(client_buffer(client_fd), "%s %s %[^\r\n]", req.method, req.uri,
               req.version) != 3) {
      return -1;
    }
    char *abs_path = concat(2, pubdir, req.uri);

    if (strncmp(req.method, "GET", 3) != 0) {
      header_bytes =
          snprintf(client_buffer(client_fd), BUFFER_SIZE, REPLY_405, date);
      printf("Server only support `GET` method with client_fd %d\n", client_fd);
    } else if (access(abs_path, F_OK) != 0) {
      header_bytes =
          snprintf(client_buffer(client_fd), BUFFER_SIZE, REPLY_404, date);
      printf("File not found with client_fd %d\n", client_fd);
    } else if (access(abs_path, R_OK) == -1) {
      header_bytes =
          snprintf(client_buffer(client_fd), BUFFER_SIZE, REPLY_403, date);
      printf("Can`t read file with client_fd %d\n", client_fd);
    } else {
      file_t *file = get_file(abs_path);
      if (file == NULL) {
        header_bytes =
            snprintf(client_buffer(client_fd), BUFFER_SIZE, REPLY_404, date);
        printf("Get_file problem with client_fd %d (HTTP 404)\n", client_fd);
      } else {
        header_bytes = snprintf(client_buffer(client_fd), BUFFER_SIZE,
                                REPLY_200, date, file->size);
        if (header_bytes >= BUFFER_SIZE)
          die("Resp headers too long!");
        // set end of client buffer
        file_fds[client_fd] = file->fd;
        file_length[client_fd] = file->size;
      }
      free(file);
    }
  } else {
    return EXIT_FAILURE;
  }
exit:
  buffer_lengths[client_fd] = 0;
  *(client_buffer(client_fd) + header_bytes) = '\0';
  resp_header_length[client_fd] = header_bytes;
  return EXIT_SUCCESS;
}

void read_buffer_from_client(int client_fd, ssize_t *bytes_read) {
  ssize_t current_length = buffer_lengths[client_fd];
  // printf("EPOLLIN triggered\n");
  // printf("client IP: %s, port %d\n", inet_ntoa(peer_addr.sin_addr),
  // peer_addr.sin_port);
  *bytes_read = read(
      client_fd, client_buffer(client_fd) + current_length,
      BUFFER_SIZE - current_length); // client_buffer(client_fd)+current_length
  if ((*bytes_read) < 0) {
    if (errno == EAGAIN && errno == EWOULDBLOCK) {
      return;
    } else {
      perror("read");
      return;
    }
  } else if ((*bytes_read) == 0) {
    // EOF encountered
    return;
  }
}

void send_buffer_to_client(int client_fd) {

  ssize_t current_length = buffer_lengths[client_fd];
  ssize_t writen = write(client_fd, client_buffer(client_fd) + current_length,
                         resp_header_length[client_fd] - current_length);
  if (writen < 0) {
    if (errno == EAGAIN && errno == EWOULDBLOCK)
      return;
    else {
      perror("write");
      // buffer_lengths[client_fd] = 0;
      return;
    }
  } else if (writen == 0) {
    // buffer_lengths[client_fd] = 0;
    return;
  }
  buffer_lengths[client_fd] += writen;
}

void send_file_to_client(int client_fd) {
  int send_fd = file_fds[client_fd];
  ssize_t size_send_fd = file_length[client_fd];
  size_send_fd = sendfile(client_fd, send_fd, NULL, size_send_fd);
  if (size_send_fd < 0) {
    if (errno == EAGAIN && errno == EWOULDBLOCK)
      return;
    else {
      perror("sendfile");
      file_length[client_fd] = 0;
      return;
    }
  } else if (size_send_fd == 0) {
    file_length[client_fd] = 0;
    return;
  }
  file_length[client_fd] -= size_send_fd;
}

void *event_loop(void *args) {
  int host = ((thread_args *)args)->host;
  u_int16_t port = ((thread_args *)args)->port;

  int listen_sock_fd;
  struct sockaddr_in addr;
  struct epoll_event events[MAX_EVENTS];
  int fds_num;
  int client_fd;
  // if epoll_wait timeout = 0 (i.e. busy wait), response rate increses a bit,
  // but CPU usage increases as well, therefore consider using timeout = -1
  int ep_timeout = -1;

  // create socket
  listen_sock_fd =
      socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  // printf("socket_fd: %d\n", listen_sock_fd);
  if (listen_sock_fd == -1) {
    die("socket failed");
  }

  set_listener_socket_opts(listen_sock_fd);

  // prepare address
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = host;
  addr.sin_port = htons(port);

  if (bind(listen_sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    die("bind failed");
  }
  if (listen(listen_sock_fd, SOMAXCONN) == -1) {
    die("listen failed");
  }

  // add listen_sock_fd to epoll
  add_efd_to_epoll(epoll_fd, listen_sock_fd, STATUS_ACCEPT,
                   EPOLLIN | EPOLLEXCLUSIVE);

  while (1) {
    fds_num = epoll_wait(epoll_fd, events, MAX_EVENTS, ep_timeout);
    if (fds_num == -1) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < fds_num; i++) {
      uint64_t client_status = (uint64_t)events[i].data.ptr;

      if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
        del_fd_from_epoll(epoll_fd, unpack_client_fd(client_status));
      }

      switch (unpack_client_status(client_status)) {
      case STATUS_ACCEPT: // generates client_fd
        client_fd =
            accept4(listen_sock_fd, NULL, NULL, SOCK_CLOEXEC | O_NONBLOCK);
        if (client_fd < 0) {
          if ((errno == EAGAIN && errno == EWOULDBLOCK)) {
            break;
          } else {
            die("accept4(listen_sock_fd) failed");
          }
        } else {
          // printf("client num %d / %d with fd %d trying to accept\n",
          // client_fd - 3 - THREADS_NUM, MAX_CONNECTIONS, client_fd);
          if ((client_fd - 3 - THREADS_NUM) > MAX_CONNECTIONS) {
            printf("server capacity exceeded %d / %d \n",
                   client_fd - 3 - THREADS_NUM, MAX_CONNECTIONS);
            close(client_fd);
          } else {
            // printf("client fd %d is accepted\n", client_fd);
            // pid_t tid = gettid();
            // printf("Accept on thread %d, client_fd: %d\n", tid, client_fd);

            // setup start buffer length & resp_header_length & file_length &
            // file_fds
            buffer_lengths[client_fd] = 0;
            resp_header_length[client_fd] = 0;
            file_length[client_fd] = 0;
            file_fds[client_fd] = 0;

            // add conn_fd to epoll
            add_efd_to_epoll(epoll_fd, client_fd, STATUS_READ,
                             EPOLLIN | EPOLLET | EPOLLONESHOT); //
            // printf("Added to efd client_fd: %d\n", client_fd);
          }
        }
        break;

      case STATUS_READ: // needed client_fd
        client_fd = unpack_client_fd(client_status);

        ssize_t ret;
        read_buffer_from_client(client_fd, &ret);
        if (ret) {
          int result = handle_request(client_fd, ret);

          if (result == EXIT_SUCCESS)
            reload_efd_in_epoll(epoll_fd, client_fd, STATUS_WRITE,
                                EPOLLOUT | EPOLLET | EPOLLONESHOT);
          else if (result == EXIT_FAILURE) {
            reload_efd_in_epoll(epoll_fd, client_fd, STATUS_READ,
                                EPOLLIN | EPOLLET | EPOLLONESHOT);
          } else { //(result == -1)
            printf("Ignoring incorrect (malformed) http request\n");
            del_fd_from_epoll(epoll_fd, client_fd);
            close(client_fd);
          }
        } else {
          del_fd_from_epoll(epoll_fd, client_fd);
          close(client_fd);
        }

        break;

      case STATUS_WRITE: // needed client_fd
        // printf("client fd ready to write\n");
        client_fd = unpack_client_fd(client_status);
        // if we have to write do write
        if (buffer_lengths[client_fd] < resp_header_length[client_fd]) {
          send_buffer_to_client(client_fd);
        }
        //Если не отправили все что нужно, перевзводим флаг и заканчиваем
        //итерацию
        if (buffer_lengths[client_fd] < resp_header_length[client_fd]) {
          reload_efd_in_epoll(epoll_fd, client_fd, STATUS_WRITE,
                              EPOLLOUT | EPOLLET | EPOLLONESHOT);
        } else {
          // buffer sended, now we need to send file if exists
          if (file_fds[client_fd] && (file_length[client_fd] != 0)) {
            send_file_to_client(client_fd);
            if (file_length[client_fd] != 0) {
              reload_efd_in_epoll(epoll_fd, client_fd, STATUS_WRITE,
                                  EPOLLOUT | EPOLLET | EPOLLONESHOT);
            } else {
              del_fd_from_epoll(epoll_fd, client_fd);
              close(file_fds[client_fd]);
              close(client_fd);
            }
          } else { // header was sended and file not`t need to send -> do clear
            del_fd_from_epoll(epoll_fd, client_fd);
            if (file_fds[client_fd])
              close(file_fds[client_fd]);
            close(client_fd);
          }
        }
        break;
      default:
        break;
      }
    } // for fds_num
  }
  // close fds
  close(epoll_fd);
  close(listen_sock_fd);
  return NULL;
}

void thread_loop(void *args) {
  pthread_t threads[THREADS_NUM];
  for (int i = 0; i < THREADS_NUM; ++i) {
    if (pthread_create(&threads[i], NULL, event_loop, args) != 0) {
      printf("pthread_create(%d) failed", i);
      exit(EXIT_FAILURE);
    }
  }
  for (int i = 0; i < THREADS_NUM; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "pthread_join(%d) failed", i);
      exit(EXIT_FAILURE);
    }
  }
}

void setup_buffers(int max_connections) {
  buffers = malloc((max_connections)*BUFFER_SIZE);
  if (!buffers)
    die("malloc on buffers");

  buffer_lengths = malloc((max_connections) * sizeof(ssize_t));
  if (!buffer_lengths)
    die("malloc on buffer_lengths");

  resp_header_length = malloc((max_connections) * sizeof(ssize_t));
  if (!resp_header_length)
    die("malloc on resp_header_length");

  file_fds = malloc((max_connections) * sizeof(int));
  if (!file_fds)
    die("malloc on file_fds");

  file_length = malloc((max_connections + 5) * sizeof(int));
  if (!file_length)
    die("malloc on file_length");
}

void setup_server() {
  setup_rlimit_nofile(MAX_OPEN_FILES_DESCRIPTORS);

  setup_buffers(MAX_CONNECTIONS * 2 + THREADS_NUM +
                3); // for max client_id index

  epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd == -1) {
    die("epoll_create failed");
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Use ./http_server <workdir> <ip> <port>\n");
    exit(EXIT_SUCCESS);
  }
  pubdir = argv[1];
  DIR *dir = opendir(pubdir);
  if (dir) {
    /* Directory exists. */
    closedir(dir);
  } else if (ENOENT == errno) {
    die("Base directory not found!\n");
  } else {
    die("Problem with opendir\n");
  }
  setup_server();

  thread_args server_args;
  server_args.host = inet_addr(argv[2]);
  server_args.port = atoi(argv[3]);
  printf("Startring server on %s:%s\n", argv[2], argv[3]);
  thread_loop(&server_args);
  close(epoll_fd);
}
