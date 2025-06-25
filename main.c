#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void show_invitation(int);

int main() {
  char c;
  char curstr[255];
  char oparg[255];
  char prev_c;
  int res_status = 0;
  int pwtype = WORD;
  char opsymb[3];
  flags_t flags;

  char rargs[2][255]; // <, (> || >>)

  word_item_t *wcur = NULL;
  word_item_t *wstart = NULL;
  word_item_t tmp;

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
          fprintf(
              stderr,
              "Error: please provide a correct argument after special token\n");
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
        case WINBG:
          if (flags.nlin) {
            flags.bg = 1;
          } else {
            fprintf(stderr, "Error: not correct use of &\n");
            flags.err = 1;
          }
          break;
        case WINP:
          flags.inp = 1;
          flags.warg = 1;
          if (flags.pip) {
            flags.err = 1;
            fprintf(stderr, "Error: input redirection \
can be used only in the first chain of pipe!\n");
          }
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
            fprintf(stderr, "Error: stdout is already redirected\n");
          } else {
            flags.out = 1;
            flags.warg = 1;
          }
          break;
        case EXT:
          goto exit;
        case WPIP:
          if (flags.oua || flags.out) {
            flags.err = 1;
            fprintf(stderr, "Error: output redirection \
can be used only in the last chain of pipe!\n");
            break;
          } else {
            l_add(&wcur, &wstart, curstr, wlen);
            flags.pip = 1;
          }
          break;
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
        if (flags.pip) {
          res_status = run_pipes(wstart, &flags, rargs);
        } else {
          int in_out[2];
          in_out[0] = -1;
          in_out[1] = -1;
          char **argv;
          convlist(wstart, &argv);
          res_status = run_prog(argv, &flags, rargs, in_out);
        }
        if (WIFEXITED(res_status)) {
          res_status = WEXITSTATUS(res_status);
        } else {
          res_status = WTERMSIG(res_status);
        }
      }

      flags.par_used = 0; // reset "parentheses were used" flag
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

void show_invitation(int status) {
  fflush(stderr);
  printf("%d>", status);
}