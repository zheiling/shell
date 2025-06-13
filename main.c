#include "main.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void show_invitation(int);
int convlist(word_item *lstart, char ***argv);
int run_prog(word_item *lstart, int flags);
void reset_flags(t_flags *flags);

int main() {
  char c;
  char curstr[1024];
  char prev_c;
  int curstrpos = 0;
  int res_status = 0;
  int run_flags = 0;
  char opsymb[3];
  t_flags flags;

  word_item *wcur = NULL;
  word_item *wstart = NULL;
  word_item tmp;
  tmp.word = NULL;

  reset_flags(&flags);
  show_invitation(0);

  while ((c = getchar()) != EOF) {
    if (flags.compound && !flags.par_open) {
      switch (prev_c) {
      case '\\':
        if (c == '"' || c == '\\' || c == ' ') {
          curstr[curstrpos++] = c;
        } else {
          printf("Error: unknown symbol after escape character\n");
          res_status = 1;
          flags.reset = 1;
        }
        break;
      case '&':
        if (c == '&') {
          strcpy(opsymb, "&&");
          flags.not_implemented = 1;
          flags.reset = 1;
          break;
        } else {
          if (c != '\n') {
            fprintf(stderr, "Error: not correct use of &\n");
            flags.reset = 1;
            break;
          }
          run_flags |= INBACK;
          flags.skip_add_w = 1;
          break;
        }
      case '>':
        if (c == '>') {
          strcpy(opsymb, ">>");
          flags.not_implemented = 1;
          flags.reset = 1;
        } else {
          strcpy(opsymb, ">");
          flags.not_implemented = 1;
          flags.reset = 1;
        }
        break;
      case '|':
        if (c == '|') {
          strcpy(opsymb, "||");
          flags.not_implemented = 1;
          flags.reset = 1;
        } else {
          strcpy(opsymb, "|");
          flags.not_implemented = 1;
          flags.reset = 1;
        }
        break;
      }
    }

    if (!flags.reset) {
      if ((c == ' ' && !flags.par_open) || c == '\n') {
        curstr[curstrpos++] = '\0';
        if (!flags.skip_add_w) {
          l_add(&wcur, &wstart, curstr, curstrpos);
        }
        curstrpos = 0;
        if (!strcmp(curstr, "quit") || !strcmp(curstr, "exit")) {
          break;
        }
        if (c == '\n') {
          if (flags.par_open) {
            printf("Error: unmatched quotes\n");
            res_status = 1;
            flags.reset = 1;
          } else {
            res_status = run_prog(wstart, run_flags);
            reset_flags(&flags);
            run_flags = 0;
            while (!l_shift(&wstart, &tmp, &wcur))
              ;
            show_invitation(res_status);
          }
        }
      } else {
        if (c == '"') {
          flags.par_open = !flags.par_open;
          continue;
        }
        if (!flags.par_open) {
          switch (c) {
          case '\\':
          case '>':
          case '&':
          case '|':
            flags.compound = 1;
            prev_c = c;
            break;
          case '<':
            strcpy(opsymb, "<");
            flags.not_implemented = 1;
            flags.reset = 1;
            break;
          case ';':
            strcpy(opsymb, ";");
            flags.not_implemented = 1;
            flags.reset = 1;
            break;
          case '(':
            flags.c_par_open = 1;
            break;
          case ')':
            flags.c_par_open = 0;
            strcpy(opsymb, "(");
            flags.not_implemented = 1;
            flags.reset = 1;
            break;
          default:
            curstr[curstrpos++] = c;
          }
        } else {
          curstr[curstrpos++] = c;
        }
      }
    }

    if (flags.not_implemented) {
      flags.not_implemented = 0;
      fprintf(stderr, "Function \"%s\" is not implemented yet.\n", opsymb);
    }

    if (flags.reset) {
      reset_flags(&flags);
      if (c != '\n') {
        while ((c = getchar()) != '\n')
          ;
      };
      while (!l_shift(&wstart, &tmp, &wcur))
        ;

      show_invitation(res_status);
    }
  }

  if (tmp.word != NULL)
    free(tmp.word);

  return 0;
}

void reset_flags(t_flags *flags) {
  flags->par_open = 0;
  flags->compound = 0;
  flags->c_par_open = 0;
  flags->not_implemented = 0;
  flags->reset = 0;
  flags->skip_add_w = 0;
}

void show_invitation(int status) {
  int p;
  do {
    p = wait4(-1, NULL, WNOHANG, NULL);
  } while (p > 0);
  fflush(stderr);
  printf("%d>", status);
}

int run_prog(word_item *lstart, int flags) {
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

  if ((flags & INBACK) != 0) {
    int p;
    do {
      p = wait4(-1, NULL, WNOHANG, NULL);
    } while (p > 0);
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
  if ((flags & INBACK) == 0) {
    wait(&status);
  }
  free(argv);

  return status;
}

int convlist(word_item *lstart, char ***argvp) {
  if (lstart == NULL) {
    return -1;
  }

  word_item *lcurrent;

  lcurrent = lstart;
  int len;
  for (len = 1; lcurrent->next != NULL;
       len++) { // len = 2 -> +1 for NULL pointer
    lcurrent = lcurrent->next;
  }

  *argvp = malloc(sizeof(char *) * (len + 1));

  lcurrent = lstart;

  int i;
  for (i = 0; i < len; i++, lcurrent = lcurrent->next) {
    (*argvp)[i] = lcurrent->word;
  }
  (*argvp)[i] = NULL;

  return 0;
}