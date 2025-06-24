#include "main.h"
#include <bits/types/sigset_t.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

volatile static sig_atomic_t gpid = 0;

int mkprp(flags_t *flags, char rargs[2][255], int in_out[2]);
void sigchld_handler(int s);

int run_prog(char **argv, flags_t *flags, char rargs[2][255],
             int in_out[2]) {
  sigset_t mask_chld, mask_emtpy;
  sigemptyset(&mask_chld);
  sigaddset(&mask_chld, SIGCHLD);
  sigemptyset(&mask_emtpy);


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

  if (flags->bg) {
    signal(SIGCHLD, sigchld_handler);
  } else {
    signal(SIGCHLD, SIG_DFL);
    sigprocmask(SIG_SETMASK, &mask_chld, NULL);
  }

  fflush(stderr);
  int pid = fork();
  if (pid == 0) { // child process
    int prpres = mkprp(flags, rargs, in_out);
    if (!prpres) {
      execvp(argv[0], argv);
      perror(argv[0]);
      fflush(stderr);
    } else {
      errno = ENOENT;
    }
    _exit(errno);
  }

  if (in_out[0] != -1) {
    close(in_out[0]);
  }

  if (in_out[1] != -1) {
    close(in_out[1]);
  }

  int status = 0;
  if (!flags->bg) {
    waitpid(pid, &status, 0);
    signal(SIGCHLD, sigchld_handler);
    sigprocmask(SIG_SETMASK, &mask_emtpy, NULL);
  }
  free(argv);

  return status;
}

int mkprp(flags_t *flags, char rargs[2][255], int in_out[2]) {
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

  if (in_out[0] != -1) {
    dup2(in_out[0], 0);
    close(in_out[0]);
  }

  if (in_out[1] != -1) {
    dup2(in_out[1], 1);
    close(in_out[1]);
  }

  return 0;
}

void sigchld_handler(int s) {
  int s_errno = errno;
  int p;
  do {
    p = wait4(-1, NULL, WNOHANG, NULL);
  } while (p > 0);
  errno = s_errno;
}