#include "main.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int mkprp(t_flags *flags, char rargs[2][255]);

int run_prog(word_item *lstart, t_flags *flags, char rargs[2][255]) {
  char **argv;

  convlist(lstart, &argv);

  // "cd" case
  if (!strcmp(argv[0], "cd")) {
    char *a;
    if (argv[1] != NULL) {
      a = argv[1];
    } else {
      a = getenv("HOME");
      if (a == NULL) {
        fprintf(stderr, "Can't find home directory!\n");
        return -1;
      }
    }

    int res = chdir(a);
    if (res == -1) {
      perror(a);
      fflush(stderr);
      return errno;
    }
    return 0;
  }

  if (flags->bc) {
    int p;
    do {
      p = wait4(-1, NULL, WNOHANG, NULL);
    } while (p > 0);
  }

  fflush(stderr);
  int pid = fork();
  if (pid == 0) { // child process
    int prpres = mkprp(flags, rargs);
    if (!prpres) {
      execvp(argv[0], argv);
      perror(argv[0]);
      fflush(stderr);
    } else {
      errno = ENOENT;
    }
    _exit(errno);
  }

  int status = 0;
  if (!flags->bc) {
    wait(&status);
  }
  free(argv);

  return status;
}

int mkprp(t_flags *flags, char rargs[2][255]) {
  if (flags->inp) {
    int fd = open(rargs[0], O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "Error: can't open file \"%s\" for input\n", rargs[0]);
      return -1;
    }
    dup2(fd, 0);
  }

  if (flags->oua || flags->out) {
    int fd;
    if (flags->oua) {
      fd = open(rargs[1], O_WRONLY | O_APPEND | O_CREAT);
    } else if (flags->out) {
      fd = open(rargs[1], O_WRONLY | O_TRUNC | O_CREAT);
    }
    if (fd == -1) {
      fprintf(stderr, "Error: can't open file \"%s\" for output\n", rargs[1]);
      return -1;
    }
    dup2(fd, 1);
  }

  return 0;
}