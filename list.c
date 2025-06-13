#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int l_add(word_item **current, word_item **start, const char *a, int asize) {
  word_item *nitem = malloc(sizeof(word_item));
  nitem->word = malloc(asize);
  strcpy(nitem->word, a);
  nitem->next = NULL;
  if (*current != NULL) {
    (*current)->next = nitem;
  } else {
    *start = nitem;
  }
  *current = nitem;
  return 0;
}

int l_shift(word_item **src, word_item *dst, word_item **current) {
  if (*src == NULL) {
    *current = NULL;
    return 1;
  }

  if (dst->word != NULL) {
    free(dst->word);
  }

  dst->word = malloc(sizeof((*src)->word));

  strcpy(dst->word, (*src)->word);
  dst->next = (*src)->next;
  free((*src)->word);
  free((*src));
  *src = dst->next;

  return 0;
}