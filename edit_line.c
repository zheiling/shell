#include "main.h"
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int it_dir(DIR *dir, char *key, int keylen, word_item_t **sr_s,
           word_item_t **sr_c);

int p_search_by_key(char *key, int first_word) {
  DIR *dir;
  char name[256] = ".";

  word_item_t wtmp;
  wtmp.word = NULL;
  word_item_t *sr_s, *sr_c; // search result start, current
  sr_s = sr_c = NULL;
  int vnum = 0;
  int keylen = strlen(key);
  // open dir
  if (first_word && key[0] != '.' && key[0] != '/') { // program name
    char *names = getenv("PATH");
    int idx = 0;
    int nm_len = strlen(names);

    while (names[idx]) {
      int i;
      for (i = 0; names[idx] && names[idx] != ':'; idx++, i++) {
        name[i] = names[idx];
      }
      name[i] = '\0';
      if (names[idx] == ':')
        idx++;
      dir = opendir(name);
      if (!dir)
        goto error;
      vnum += it_dir(dir, key, keylen, &sr_s, &sr_c);
    }

  } else { // argument (file name)
    if (key[0] == '/') {
      dir = opendir("/");
    } else {
      dir = opendir(".");
    }
    char *keystart = key;
    if (key[0] == '.' && key[1] == '/') {
      keystart += 2;
      keylen -= 2;
    } else if (key[0] == '/') {
      keystart++;
      keylen--;
    }
    vnum += it_dir(dir, keystart, keylen, &sr_s, &sr_c);
  }

  if (vnum == 1) {
    l_shift(&sr_s, &wtmp, &sr_c);
    strcpy(key, wtmp.word);
  } else if (vnum > 1) {
    while (!l_shift(&sr_s, &wtmp, &sr_c)) {
      printf("%s\t", wtmp.word);
    }
    putchar('\n');
    free(wtmp.word);
  }

error:
  if (!dir) {
    perror(name);
    return 1;
  }

  return vnum;
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
  if (spos == 0)
    return -1;
  return spos;
}

int it_dir(DIR *dir, char *key, int keylen, word_item_t **sr_s,
           word_item_t **sr_c) {
  struct dirent *dent;
  int len = 0;
  char ctmp[MAX_LINE];
  word_item_t wtmp;
  char name[256];
  char *namen;

  while ((dent = readdir(dir)) != NULL) {
    strcpy(name, dent->d_name);
    if (strlen(name)) {
      if (!strncmp(name, key, keylen)) {
        if (find_occ(*sr_s, name))
          continue;
        namen = name; // !debug
        if (!l_add(sr_c, sr_s, dent->d_name, strlen(dent->d_name))) {
          len++;
        }
      }
    }
  }
  return len;
}