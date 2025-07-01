#include "main.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int p_search_by_key(word_item_t *start, char *key) {
  word_item_t *f_st, *f_cur;
  f_st = NULL;
  int len = l_search(start, key, &f_st);

  if (len > 0) {
    char *wk;
    f_cur = f_st;
    if (len > 1) {
      putchar('\n');
      while (!l_shift_2(&f_st, &wk, &f_cur)) {
        printf("%s\t", wk);
      }
      putchar('\n');
    } else {
      l_shift_2(&f_st, &wk, &f_cur);
      strcpy(key, wk);
    }
    free(f_st);
  }
  return len;
}

void move_cursor(int num) {
  if (num > 0) {
    for (int i = 0; i < num; i++) {
      write(1, "\b", 1);
    }
  }
}

void erase_symbols(int num) {
  if (num > 0) {
    for (int i = 0; i < num; i++) {
      write(1, "\b", 1);
    }
    for (int i = 0; i < num; i++) {
      write(1, " ", 1);
    }
    for (int i = 0; i < num; i++) {
      write(1, "\b", 1);
    }
  }
}

void del_sym(char arr[], int cur_ind, int *buf_index) {
  char c1, c2;
  int bindex = *buf_index - 1;
  int binx = bindex;

  if (binx > 1) {
    c1 = arr[binx--];
    c2 = arr[binx];
    while (cur_ind > 0 && binx >= 0) {
      arr[binx--] = c1;
      if (binx == -1) {
        break;
      }
      c1 = c2;
      c2 = arr[binx];
      cur_ind--;
    }
  }

  arr[bindex] = '\0';

  *buf_index = *buf_index - 1;
}

void ins_sym(char arr[], int cur_ind, int *buf_index, char a) {
  char c1, c2;

  int idx = *buf_index - cur_ind;

  c1 = arr[idx];
  c2 = arr[idx + 1];

  arr[idx] = a;
  idx++;
  while (idx <= *buf_index) {
    arr[idx++] = c1;
    c1 = c2;
    c2 = arr[idx];
  }

  *buf_index = *buf_index + 1;
  arr[*buf_index] = '\0';
}

int last_space(char arr[], int buf_index) {
  int spos = 0;
  for (int i = 0; i < buf_index; i++) {
    if (arr[i] == ' ')
      spos = i;
  }
  if (spos == 0) return -1;
  return spos;
}