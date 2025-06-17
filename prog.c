#include "main.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int run_prog(word_item *lstart, int flags, char rargs[3][255]) {
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

  if ((flags & WINBC) != 0) {
    int p;
    do {
      p = wait4(-1, NULL, WNOHANG, NULL);
    } while (p > 0);
  }

  if ((flags & WINP) != 0) {
    printf("<: %s\n", rargs[0]);
  }

  if ((flags & WOUA) != 0) {
    printf(">>: %s\n", rargs[1]);
  }

  if ((flags & WOUT) != 0) {
    printf(">: %s\n", rargs[1]);
  }

  fflush(stderr);
  int pid = fork();
  if (pid == 0) { // child process
    execvp(argv[0], argv);
    perror(argv[0]);
    fflush(stderr);
    _exit(errno);
  }

  int status = 0;
  if ((flags & WINBC) == 0) {
    wait(&status);
  }
  free(argv);

  return status;
}