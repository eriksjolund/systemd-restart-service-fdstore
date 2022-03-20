#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <systemd/sd-daemon.h>
#include <unistd.h>

#define STORED_FDNAME "upcase"

void upcase_string(char * str, int n) {
  for (int i = 0; i < n ; ++i) {
    str[i] = toupper(str[i]);
  }
}

void upcase_forever(int accepted_sock) { 
  char buf[1024];
  // every nth time, let's exit the program with EXIT_FAILURE so that
  // systemd will try to restart the program (and at the same time
  // pass the previously stored accepted TCP socket)
  const int nth = 3;
  int i = 0;
  while (1) {
    ++i;
    if (i % nth == 0) {
      exit(EXIT_FAILURE);
    }
    int nread, nwritten;
    nread = read(accepted_sock, buf, sizeof(buf));
    if (nread < 1) {
      perror("nread < 1");
      exit(EXIT_FAILURE);
    }
    upcase_string(buf, nread);
    nwritten = write(accepted_sock, buf, nread);
    if (nwritten != nread) {
      // To keep it simple we treat this as an error
      // although it is not an error condition
      perror("nwritten != nread");
      exit(EXIT_FAILURE);
    }
  }
}

void print_names(FILE *file,  char ** names, int num_names) {
  for (int i = 0; i < num_names; i++) {
    fprintf(file, "name %i=%s\n", i, names[i]);
  }
}

void maybe_upcase_forever( char ** names, int num_names) {
  // Run upcase_forever() if the program was started
  // with a previously stored (FDSTORE=1) accepted TCP socket
  for (int i = 0; i < num_names; i++) {
    if (strcmp(names[i], STORED_FDNAME) == 0) {
      fprintf(stderr, "using stored accepted socket\n");
      int sockfd = SD_LISTEN_FDS_START + i; 
      upcase_forever(sockfd);
    }
  }
}

int main() {
  fprintf(stderr, "main start\n");
  char** names = NULL;
  const char * upcase_str = STORED_FDNAME;
  int num_fds = sd_listen_fds_with_names(0, &names);
  if (num_fds < 1) {
    perror("return value sd_listen_fds_with_names(). An integer greater than zero is expected)");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "num_fds=%i\n", num_fds);
  print_names(stderr, names, num_fds);
  maybe_upcase_forever(names, num_fds);
  int sockfd = SD_LISTEN_FDS_START; 
  int optval;
  int optlen;
  optlen = sizeof(int);
  getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &optval, &optlen);
  if (optval == SOCK_STREAM) {
    int accepted_sock = accept(sockfd, NULL, NULL);
    if (accepted_sock == -1) {
      perror("accept() error");
      exit(EXIT_FAILURE);
    }
    char notify_str[100];
    snprintf(notify_str, sizeof(notify_str), "FDSTORE=1\nFDPOLL=0\nFDNAME=%s", STORED_FDNAME);
    int res = sd_pid_notify_with_fds(getpid(), 0, notify_str, &accepted_sock, 1);
    fprintf(stderr, "sd_pid_notify_with_fds() returned  %i\n", res);
    sd_notify_barrier(0, 1000);
    fprintf(stderr, "after sd_notify_barrier\n");
    upcase_forever(accepted_sock);
  } else {
    perror("Expected SOCK_STREAM (Please configure the systemd socket with a ListenStream= directive)");
    exit(EXIT_FAILURE);   
  }
  return 0;
}
