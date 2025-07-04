#include "main.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *log_f;

int l_add(word_item_t **current, word_item_t **start, const char *a,
          int asize) {
  word_item_t *nitem = malloc(sizeof(word_item_t));
  nitem->word = (char *)malloc(asize + 1);
  strncpy(nitem->word, a, asize);
  nitem->word[asize] = '\0';
  nitem->next = NULL;
  if (*current != NULL) {
    (*current)->next = nitem;
  } else {
    *start = nitem;
  }
  *current = nitem;
  return 0;
}

int l_shift(word_item_t **src, word_item_t *dst, word_item_t **current) {
  if (*src == NULL) {
    if (current != NULL) {
      *current = NULL;
    }
    return 1;
  }

  if (dst->word != NULL) {
    free(dst->word);
  }
  int stlen = strlen((*src)->word);
  dst->word = malloc(stlen + 1);

  if (dst->word == NULL) {
    fprintf(stderr, "Oshibka !\n");
    return 2;
  }
  strcpy(dst->word, (*src)->word);

  dst->next = (*src)->next;
  if (dst != *src) {
    free((*src)->word);
    free((*src));
  }
  *src = dst->next;

  return 0;
}

int convlist(word_item_t *lstart, char ***argvp) {
  if (lstart == NULL) {
    return -1;
  }

  word_item_t *lcurrent;

  lcurrent = lstart;
  int len;
  for (len = 1; lcurrent->next != NULL && strcmp(lcurrent->word, "|");
       len++) { // len = 2 -> +1 for NULL pointer
    lcurrent = lcurrent->next;
  }

  *argvp = malloc(sizeof(char *) * (len + 1));

  lcurrent = lstart;

  int i;
  for (i = 0; i < len; i++, lcurrent = lcurrent->next) {
    if (!strcmp(lcurrent->word, "|"))
      break;
    (*argvp)[i] = lcurrent->word;
  }
  (*argvp)[i] = NULL;

  return 0;
}

int l_add_2(word_item_t **current, word_item_t **start, char *a) {
  word_item_t *nitem = malloc(sizeof(word_item_t));
  nitem->word = a;
  nitem->next = NULL;
  if (*current != NULL) {
    (*current)->next = nitem;
  } else {
    *start = nitem;
  }
  *current = nitem;
  return 0;
}

int l_shift_2(word_item_t **src, char dst[MAX_LINE], word_item_t **current) {
  if (*src == NULL) {
    *current = NULL;
    return 1;
  }

  word_item_t *src2 = *src;
  strcpy(dst, (*src)->word);
  *src = (*src)->next;
  free((src2));
  return 0;
}

int l_search(word_item_t *start, char *key_w, word_item_t **fsstart) {
  word_item_t *ptr = start;
  int key_len = strlen(key_w);
  word_item_t *f_cur = *fsstart;
  int count = 0;

  while (ptr != NULL) {
    if (!strncmp(ptr->word, key_w, key_len)) {
      int w_len = strlen(ptr->word);
      l_add_2(&f_cur, fsstart, ptr->word);
      count++;
    }
    ptr = ptr->next;
  }
  return count;
}

// find occurences
int find_occ(word_item_t *start, char *key_w) {
  word_item_t *ptr = start;
  int key_len = strlen(key_w);

  while (ptr != NULL) {
    if (!strncmp(ptr->word, key_w, key_len)) {
      return 1;
    }
    ptr = ptr->next;
  }
  return 0;
}