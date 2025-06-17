#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int l_add(word_item **current, word_item **start, const char *a, int asize) {
  word_item *nitem = malloc(sizeof(word_item));
  nitem->word = malloc(asize);
  strncpy(nitem->word, a, asize);
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