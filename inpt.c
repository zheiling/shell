#include "main.h"
#include <stdio.h>
#include <string.h>

// TODO: использовать ungetc

int analyze_word(char word[MAX_LINE]) {
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
    return WINBG;
  if (!strcmp(word, "|"))
    return WPIP;
  if (!strcmp(word, ">"))
    return WOUT;
  if (!strcmp(word, "<"))
    return WINP;
  return WORD;
}

int extract_word(char dest[MAX_LINE], flags_t *flags, char line[MAX_LINE]) {
  char c;
  int i = 0;
  static int l_idx = 0;
  int sc = 0;
  int fi = 1;
  int openpar = 0;
  static char prev_sc = 0;
  static char prev_c = 0;

  if (!flags->nfw) {
    l_idx = 0;
    flags->nfw = 1;
  }

  if (prev_c) {
    dest[i++] = prev_c;
    prev_c = 0;
  }
  if (prev_sc)
    dest[i++] = prev_sc;

  while ((c = line[l_idx++]) == ' ')
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
    case '|':
    case '<':
    case '>':
    case '&':
      if (i != 0 && !prev_sc) {
        prev_sc = c;
        goto end_loop;
      } else if (i != 0 && prev_sc) {
        dest[i++] = c;
        prev_c = 0;
        goto end_loop;
      }
    default:
      if (!prev_sc) {
        dest[i++] = c;
      } else {
        prev_c = c;
        prev_sc = 0;
        goto end_loop;
      }
    }
  } while ((c = line[l_idx++]) && (c != ' ' || sc || openpar) && c != '\n' &&
           i < MAX_LINE);

end_loop:
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
    l_idx = 0;
  }
  flags->err = 1;
  return -1;
}