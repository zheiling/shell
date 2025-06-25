#include "main.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct pipe_t {
  char **val;
  struct pipe_t *next;
} pipe_l_t;

int mconvlist(word_item_t *lstart, char ***argvp);
void lp_add(pipe_l_t **current, pipe_l_t **start, char **val);
pipe_l_t *iterate_list(flags_t *flags, word_item_t *lstart);
char **lp_shift(pipe_l_t **src);

void sigpipehandler(int s) { exit(0); }

int run_pipes(word_item_t *lstart, flags_t *flags, char rargs[2][255]) {
  pipe_l_t *pl_start = iterate_list(flags, lstart);
  char **vargs;
  int fd[2];
  int fd_args[2];

  // first iteration
  vargs = lp_shift(&pl_start);
  // TODO: вход неизменённый
  pipe(fd);
  fd_args[0] = -1;
  fd_args[1] = fd[1];

  int pid = fork();
  if (pid == 0) { // intermediate child process, that keeps the pipe open
    close(fd[0]);
    flags_t n_flags;
    reset_flags(&n_flags);
    n_flags.inp = flags->inp;
    run_prog(vargs, &n_flags, rargs, fd_args); // TODO: поменять rargs
    signal(SIGPIPE, sigpipehandler);
    pause();
  }

  close(fd[1]); // close output

  // middle iterations
  while (pl_start->next != NULL) {
    vargs = lp_shift(&pl_start);
    int pr_outpt = fd[0];
    pr_outpt = fd[0];
    pipe(fd);
    fd_args[0] = pr_outpt;
    fd_args[1] = fd[1];

    pid = fork();
    if (pid == 0) { // intermediate child process, that keeps the pipe open
      close(fd[0]);
      flags_t n_flags;
      reset_flags(&n_flags);
      run_prog(vargs, &n_flags, NULL, fd_args); // TODO: поменять rargs
      signal(SIGPIPE, sigpipehandler);
      pause();
    }
    close(fd[1]); // close output
  }

  // last iteration
  vargs = lp_shift(&pl_start);
  fd_args[1] = -1;
  fd_args[0] = fd[0];

  flags_t n_flags;
  reset_flags(&n_flags);
  n_flags.oua = flags->oua;
  n_flags.out = flags->out;
  n_flags.bg  = flags->bg;

  run_prog(vargs, &n_flags, rargs, fd_args); // TODO: поменять rargs

  close(fd[0]); // close input

  return 0;
}

pipe_l_t *iterate_list(flags_t *flags, word_item_t *lstart) {
  word_item_t tmp;
  word_item_t *wcurrent;
  word_item_t *wstart;
  int reset_on_next = 0;

  wstart = lstart;
  wcurrent = lstart;

  pipe_l_t *p_start = NULL;
  pipe_l_t *p_current = NULL;

  char **vargs;
  do {
    if (reset_on_next) {
      wstart = wcurrent;
      reset_on_next = 0;
    }
    if (!strcmp(wcurrent->word, "|") || wcurrent->next == NULL) {
      reset_on_next = 1;
      convlist(wstart, &vargs);
      lp_add(&p_current, &p_start, vargs);
    }
  } while ((wcurrent = wcurrent->next) != NULL);

  return p_start;
}

void lp_add(pipe_l_t **current, pipe_l_t **start, char **val) {
  pipe_l_t *nitem = malloc(sizeof(pipe_l_t));
  nitem->next = NULL;
  nitem->val = val;
  if (*current != NULL) {
    (*current)->next = nitem;
  } else {
    *start = nitem;
  }
  *current = nitem;
}

char **lp_shift(pipe_l_t **src) {
  char **val;
  pipe_l_t *src_next;

  if (*src == NULL) {
    return NULL;
  }

  val = (*src)->val;
  src_next = (*src)->next;
  free((*src));
  *src = src_next;

  return val;
}