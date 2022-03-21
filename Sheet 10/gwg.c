#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <fcntl.h>
#include "player.h"
#include "chlng.h"

extern char *optarg;

ssize_t tcp_read(int fd, void *buf, size_t count) {
  size_t nread = 0;
  int flags;

  flags = fcntl(fd, F_GETFD);
  if (flags == -1) {
    return -1;
  }

  while (count > 0) {
    int r = read(fd, buf, count);
    if (r < 0 && errno == EINTR) {
      continue;
    }
    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return nread;
    }
    if (r < 0) {
      return r;
    }
    if (r == 0) {
      return nread;
    }
    buf = (unsigned char *) buf + r;
    count -= r;
    nread += r;
    if ((flags & O_NONBLOCK) == 0) {
      return nread;
    }
  }

  return nread;
}

ssize_t tcp_write(int fd, const void *buf, size_t count) {
  size_t nwritten = 0;
  int flags;

  flags = fcntl(fd, F_GETFD);
  if (flags == -1) {
    perror("fcntl()");
    return -1;
  }


  while (count > 0) {
    int r = write(fd, buf, count);
    if (r < 0 && errno == EINTR) {
      continue;
    }
    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return nwritten;
    }
    if (r < 0) {
      return r;
    }
    if (r == 0) {
      return nwritten;
    }
    buf = (unsigned char *) buf + r;
    count -= r;
    nwritten += r;
    if ((flags & O_NONBLOCK) == 0) {
      return nwritten;
    }
  }

  return nwritten;
}

int tcp_listen(const char *host, const char *port) {
    struct addrinfo hints, *ai_list, *ai;
    int rc, fd = 0, on = 1;
    
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    rc = getaddrinfo(host, port, &hints, &ai_list);
    
    if (rc) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    for (ai = ai_list; ai; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            continue;
        }

        #ifdef IPV6_V6ONLY
        if (ai->ai_family == AF_INET6) {
            (void) setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        }
        #endif

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
            break;
        }

        (void) close(fd);
    }

    freeaddrinfo(ai_list);
    
    if (ai == NULL) {
        fprintf(stderr, "failed to bind to '%s' port %s\n", host, port);
        return -1;
    }

    if (listen(fd, 42) < 0) {
        perror("listen");
        (void) close(fd);
        return -1;
    }
    
    return fd;
}

void play_quiz(int client_socket_fd) {
  player_t *p;
  char *msg;
  int rc;

  p = player_new();
  if (!p) {
    puts("Could not allocate player");
    return;
  }
  
  rc = player_get_greeting(p, &msg);
  
  if (rc > 0) {
    tcp_write(client_socket_fd, msg, rc);
    free(msg);
  }
  
  while (! (p->state & PLAYER_STATE_FINISHED)) {
    char *msg = NULL;
    char line[1024];
    
    if (! (p->state & PLAYER_STATE_CONTINUE)) {
      player_fetch_chlng(p);
    }
    
    rc = player_get_challenge(p, &msg);
    
    if (rc > 0) {
      tcp_write(client_socket_fd, msg, rc);
      free(msg);
    }
    
    rc = tcp_read(client_socket_fd, line, 1024);
    // socket closed
    if (rc <= 0) {
      break;
    }
    
    rc = player_post_challenge(p, line, &msg);
    if (rc > 0) {
      tcp_write(client_socket_fd, msg, rc);
      free(msg);
    }
  }

  close(client_socket_fd);
  player_del(p);
}

int main(int argc, char *argv[]) {
  char opt;
  char *port = "8080";
  char *host4 = "0.0.0.0";
  char *host6 = "::";

  while((opt = getopt(argc, argv, "p:")) != -1) {
    if(opt == 'p') {
      port = optarg;
    }
  }

  int server_socket4_fd = tcp_listen(host4, port);
  int server_socket6_fd = tcp_listen(host6, port);

  int bigger_fd = server_socket6_fd > server_socket4_fd ? server_socket6_fd + 1 : server_socket4_fd + 1;
  int client_socket_fd;
  
  while(1) {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(server_socket4_fd, &rfds);
    FD_SET(server_socket6_fd, &rfds);

    int rc = select(bigger_fd, &rfds, NULL, NULL, NULL);

    if(rc == -1) {
      perror("Select: ");
      exit(EXIT_FAILURE);
    }

    if(FD_ISSET(server_socket4_fd, &rfds)) {
      client_socket_fd = accept(server_socket4_fd, NULL, NULL);
    } else if(FD_ISSET(server_socket6_fd, &rfds)) {
      client_socket_fd = accept(server_socket6_fd, NULL, NULL);
    }

    if(client_socket_fd == -1) {
      perror("Client socket creation, accept: ");
      continue;
    }

    choose_mode(client_socket_fd, quiz_mode);
  }
  
  return EXIT_SUCCESS;
}
