#include "main.h"

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
