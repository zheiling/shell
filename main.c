#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void show_invitation(int);
void reset_flags(t_flags *flags);

int main() {
  char c;
  char curstr[255];
  char oparg[255];
  char prev_c;
  int res_status = 0;
  int pwtype = WORD;
  char opsymb[3];
  t_flags flags;

  char rargs[2][255]; // <, (> || >>)

  word_item *wcur = NULL;
  word_item *wstart = NULL;
  word_item tmp;

  tmp.word = NULL;

  reset_flags(&flags);
  show_invitation(0);

  int wlen = 0;

  while ((wlen = extract_word(curstr, &flags)) != EOF) {
    if (!flags.err) {
      int atype;
      atype = analyze_word(curstr);
      if (flags.warg) {
        if (atype != WORD) {
          fprintf(stderr,
                  "Error: please provide a correct arg after special token\n");
          flags.err = 1;
          flags.inv = 1;
        } else {
          flags.warg = 0;
          switch (pwtype) {
          case WINP:
            strcpy(rargs[0], curstr);
            break;
          case WOUT:
          case WOUA:
            strcpy(rargs[1], curstr);
            break;
          }
        }
      } else if (!flags.par_used && !flags.err) {
        pwtype = atype;
        switch (atype) {
        case WINBC:
          if (flags.nlin) {
            flags.bc = 1;
          } else {
            fprintf(stderr, "Error: not correct use of &\n");
            flags.inv = 1;
          }
          break;
        case WINP:
          flags.inp = 1;
          flags.warg = 1;
          break;
        case WOUA:
          if (flags.oua || flags.out) {
            flags.err = 1;
            flags.inv = 1;
            fprintf(stderr, "Error: stdout is already redirected\n");
          } else {
            flags.oua = 1;
            flags.warg = 1;
          }
          break;
        case WOUT:
          if (flags.oua || flags.out) {
            flags.err = 1;
            flags.inv = 1;
            fprintf(stderr, "Error: stdout is already redirected\n");
          } else {
            flags.out = 1;
            flags.warg = 1;
          }
          break;
        case EXT:
          goto exit;
        case WORD:
          l_add(&wcur, &wstart, curstr, wlen);
          break;
        default:
          printf("Error: feature \"%s\" is not implemented yet\n", curstr);
        }
      } else {
        l_add(&wcur, &wstart, curstr, wlen);
      }

      if (flags.nlin && !flags.err) {
        flags.inv = 1;
        res_status = run_prog(wstart, &flags, rargs);
        if (WIFEXITED(res_status)) {
          res_status = WEXITSTATUS(res_status);
        } else {
          res_status = WTERMSIG(res_status);
        }
      }
    }

    if (flags.inv || flags.err) {
      while (!l_shift(&wstart, &tmp, &wcur))
        ;
      if (!flags.nlin) {
        clear_buf();
      }
      reset_flags(&flags);
      show_invitation(res_status);
    }
  }
exit:
  if (tmp.word != NULL)
    free(tmp.word);

  return 0;
}

void reset_flags(t_flags *flags) {
  flags->par_used = 0;
  flags->inv = 0;
  flags->nlin = 0;
  flags->err = 0;
  flags->oua = 0;
  flags->out = 0;
  flags->inp = 0;
  flags->warg = 0;
  flags->bc = 0;
}

void show_invitation(int status) {
  fflush(stderr);
  printf("%d>", status);
}