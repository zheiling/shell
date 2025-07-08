#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int res_status;

int process_line(char commline[MAX_LINE], word_item_t **arg_list, flags_t *flags,
                 char rargs[2][MAX_LINE]) {
  char curstr[MAX_LINE];
  int pwtype = WORD;

  word_item_t *wcur = NULL;
  word_item_t tmp;

  tmp.word = NULL;

  reset_flags(flags);

  int wlen = 0;

  while ((wlen = extract_word(curstr, flags, commline)) != EOF) {
    if (!flags->err) {
      int atype;
      atype = analyze_word(curstr);
      if (flags->warg) {
        if (atype != WORD) {
          fprintf(
              stderr,
              "Error: please provide a correct argument after special token\n");
          flags->err = 1;
          flags->inv = 1;
        } else {
          flags->warg = 0;
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
      } else if (!flags->par_used && !flags->err) {
        pwtype = atype;
        switch (atype) {
        case WINBG:
          if (flags->nlin) {
            flags->bg = 1;
          } else {
            fprintf(stderr, "Error: not correct use of &\n");
            flags->err = 1;
          }
          break;
        case WINP:
          flags->inp = 1;
          flags->warg = 1;
          if (flags->pip) {
            flags->err = 1;
            fprintf(stderr, "Error: input redirection \
can be used only in the first chain of pipe!\n");
          }
          break;
        case WOUA:
          if (flags->oua || flags->out) {
            flags->err = 1;
            flags->inv = 1;
            fprintf(stderr, "Error: stdout is already redirected\n");
          } else {
            flags->oua = 1;
            flags->warg = 1;
          }
          break;
        case WOUT:
          if (flags->oua || flags->out) {
            flags->err = 1;
            fprintf(stderr, "Error: stdout is already redirected\n");
          } else {
            flags->out = 1;
            flags->warg = 1;
          }
          break;
        case EXT:
          putchar('\n');
          exit(0);
        case WPIP:
          if (flags->oua || flags->out) {
            flags->err = 1;
            fprintf(stderr, "Error: output redirection \
can be used only in the last chain of pipe!\n");
            break;
          } else {
            l_add(&wcur, arg_list, curstr, wlen);
            flags->pip = 1;
          }
          break;
        case WORD:
          l_add(&wcur, arg_list, curstr, wlen);
          break;
        default:
          printf("Error: feature \"%s\" is not implemented yet\n", curstr);
        }
      } else {
        l_add(&wcur, arg_list, curstr, wlen);
      }

      if (flags->nlin || flags->err) {
        goto exit;
      }

      flags->par_used = 0; // reset "parentheses were used" flag
    }
  }
exit:
  if (tmp.word != NULL)
    free(tmp.word);

  return 0;
}