#include "main.h"
#include <stdio.h>
#include <string.h>

int analyze_word(char word[255]) {
  if (!strcmp(word, "&&"))
    return WAND;
  if (!strcmp(word, ">>"))
    return WOUA;
  if (!strcmp(word, "exit"))
    return EXT;
  if (!strcmp(word, "quit"))
    return EXT;
  if (!strcmp(word, "q"))
    return EXT;
  if (!strcmp(word, "&"))
    return WINBC;
  if (!strcmp(word, "|"))
    return WPIP;
  if (!strcmp(word, ">"))
    return WOUT;
  if (!strcmp(word, "<"))
    return WINP;
  return WORD;
}

int extract_word(char dest[255], t_flags *flags) {
  char c;
  int i = 0;
  int sc = 0;
  int fi = 1;
  int openpar = 0;

  while ((c = getchar()) == ' ')
    ;

  do {
    if (sc) {
      sc = 0;
      if (c == ' ' || c == '\\' || c == '"') {
        dest[i++] = c;
        continue;
      } else {
        printf("Error: unknown symbol \"%c\" after escape character\n", c);
        goto err;
      }
    }
    switch (c) {
    case '"':
      openpar = !openpar;
      flags->par_used = 1;
      break;
    case '\\':
      sc = 1;
      break;
    default:
      dest[i++] = c;
    }
  } while ((c = getchar()) && (c != ' ' || sc || openpar) && c != '\n' &&
           i < 255);

  dest[i++] = '\0';

  if (openpar) {
    printf("Error: unmatched quotes\n");
    goto err;
  }

  switch (c) {
  case EOF:
    return EOF;
  case '\n':
    flags->nlin = 1;
  default:
    return i;
  }
err:
  if (c != '\n') {
    clear_buf();
  }
  flags->err = 1;
  return -1;
}

void clear_buf() {
  char c;
  while ((c = getchar() != '\n'))
    ;
}