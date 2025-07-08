#include "main.h"
#include <stdio.h>

void reset_flags(flags_t *flags) {
  flags->par_used = 0;
  flags->inv = 0;
  flags->nlin = 0;
  flags->err = 0;
  flags->oua = 0;
  flags->out = 0;
  flags->inp = 0;
  flags->warg = 0;
  flags->bg = 0;
  flags->pip = 0;
  flags->nfw = 0;
}

extern int res_status;

void show_invitation() {
  fflush(stderr);
  printf("%d>", res_status);
  fflush(stdout);
}

