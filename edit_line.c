#include "main.h"
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

extern FILE *log_f;
int analyze_key(char *key, char *dirname, char *filename);

int it_dir(char *name, char *key, int keylen, word_item_t **sr_s,
           word_item_t **sr_c);

int p_search_by_key(char *key, int first_word, word_item_t **wptr) {
  char dirname[MAX_INPUT] = ".";
  char filename[MAX_INPUT];

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
        dirname[i] = names[idx];
      }
      dirname[i] = '\0';
      if (names[idx] == ':')
        idx++;
      vnum += it_dir(dirname, key, keylen, &sr_s, &sr_c);
    }
  } else { // argument (file name)
    keylen = analyze_key(key, dirname, filename);
    vnum += it_dir(dirname, filename, keylen, &sr_s, &sr_c);
  }

  *wptr = sr_s;

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

// Iterate through the directory
int it_dir(char *name, char *key, int keylen, word_item_t **sr_s,
           word_item_t **sr_c) {
  struct dirent *dent;
  int len = 0;
  char ctmp[MAX_LINE];
  word_item_t wtmp;
  char fname[MAX_LINE];
  char fname_l[MAX_LINE];
  DIR *dir;
  char key_l[MAX_LINE];

  strcpy(key_l, key);

  for (int i = 0; key_l[i] != '\0'; i++) {
    key_l[i] = tolower(key_l[i]);
  }

  dir = opendir(name);

  if (!dir) {
    perror(name);
    return -1;
  }

  while ((dent = readdir(dir)) != NULL) {
    strcpy(fname, dent->d_name);
    if (strlen(fname)) {
      strcpy(fname_l, dent->d_name);

      for (int i = 0; fname_l[i] != '\0'; i++) {
        fname_l[i] = tolower(fname_l[i]);
      }

      if (!strncmp(fname_l, key_l, keylen)) {
        if (find_occ(*sr_s, fname))
          continue;

        int fnmlen = strlen(fname);
        if (dent->d_type == DT_DIR) {
          fname[fnmlen++] = '/';
          fname[fnmlen] = '\0';
        }
        if (!l_add(sr_c, sr_s, fname, fnmlen))
          len++;
      }
    }
  }
  closedir(dir);
  return len;
}

int analyze_key(char *key, char *dirname, char *filename) {
  int d_end_index = -1;
  int idx;
  int klen = 0;

  for (idx = 0; key[idx] != '\0'; idx++) {
    if (key[idx] == '/')
      d_end_index = idx;
  }

  if (d_end_index >= 0) {
    strncpy(dirname, key, d_end_index + 1);

    if (idx > d_end_index) {
      strcpy(filename, key + d_end_index + 1);
      klen = strlen(key + d_end_index + 1);
    } else {
      filename[0] = '\0';
    }
  } else {
    strcpy(filename, key);
    klen = strlen(key);
    strcpy(dirname, ".");
  }

  return klen;
}