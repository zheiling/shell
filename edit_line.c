#include "main.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
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

int edit_line(char buf[MAX_LINE]) {
  char cs[8];
  int buf_index = 0;
  int nw_pos = 0;
  int cur_ind = 0;      // cursor indent
  word_item_t wtmp;     // temporary word_item
  word_item_t *wtmpptr; // temporary word_item pointer
  wtmpptr = NULL;
  wtmp.word = NULL;

  while ((read(0, cs, 8)) > 0) {
    switch (cs[0]) {
    case 3:
    case 4:
    case 26:
    case 28:
      return 0;
    case 8:
    case 127: // DEL
      if (!cur_ind) {
        if (buf_index > 0) {
          write(1, "\b \b", sizeof("\b \b"));
          buf_index--;
        }
      } else {
        erase_symbols(buf_index);
        del_sym(buf, cur_ind, &buf_index);
        write(0, buf, buf_index);
        move_cursor(cur_ind);
      }
      nw_pos = last_space(buf, buf_index) + 1;
      break;
    case 9:
      // tab logic
      if (buf_index == 0) {
        buf[buf_index++] = '\t';
        write(0, buf, buf_index);
        break;
      }
      buf[buf_index] = '\0';
      int word_start = !nw_pos;
      int len = p_search_by_key(buf + nw_pos, word_start, &wtmpptr);
      if (len == 0)
        break;

      erase_symbols(buf_index);
      if (len == 1) {
        l_shift(&wtmpptr, &wtmp, NULL);
        int slashpos = -1;

        for (int i = 0; buf[i] != '\0'; i++) {
          if (buf[i] == '/')
            slashpos = i;
        }

        if (slashpos >= 0) {
          strcpy(buf + slashpos + 1, wtmp.word);
        } else {
          strcpy(buf + nw_pos, wtmp.word);
        }
        buf_index = strlen(buf);
        if (buf[buf_index - 1] != '/') {
          nw_pos = buf_index;
        }
      } else {
        while (!l_shift(&wtmpptr, &wtmp, NULL)) {
          printf("%s\t", wtmp.word);
          fflush(stdout);
        }
        putchar('\n');
        show_invitation();
      }
      write(0, buf, buf_index);
      break;
    case 10:
      buf[buf_index++] = '\n';
      putchar('\n');
      goto exit;
      break;
    case 23: // Ctrl-W
      if (buf_index > 0) {
        if (buf[buf_index - 1] == ' ') {
          buf_index--;
          nw_pos = last_space(buf, buf_index) + 1;
          erase_symbols(1);
        }
        if (nw_pos > 0) {
          erase_symbols(buf_index - nw_pos);
          buf_index = nw_pos;
        } else {
          erase_symbols(buf_index);
          buf_index = 0;
        }
      }
      break;
    case 27: {
      if (cs[1] == 91) {
        switch (cs[2]) {
        case 68: // <-
          move_cursor(1);
          cur_ind++;
          break;
        case 67: // ->
          if (cur_ind > 0) {
            erase_symbols(buf_index);
            write(0, buf, buf_index);
            cur_ind--;
            move_cursor(cur_ind);
          }
          break; // local
        }
        break;
      }
    }
    default:
      if (!cur_ind) {
        buf[buf_index++] = cs[0];
        write(1, cs, 1);
      } else {
        erase_symbols(buf_index);
        ins_sym(buf, cur_ind, &buf_index, cs[0]);
        write(0, buf, buf_index);
        move_cursor(cur_ind);
      }
      if (cs[0] == ' ')
        nw_pos = buf_index;
    }

    for (int i = 0; i < 8; i++) {
      cs[i] = 0;
    }
  }

exit:
  if (errno) {
    return errno;
  }

  return 0;
}

int p_search_by_key(char *key, int first_word, word_item_t **wptr) {
  char dirname[MAX_INPUT] = ".";
  char filename[MAX_INPUT];

  word_item_t *sr_s, *sr_c; // search result start, current
  sr_s = sr_c = NULL;
  int vnum = 0;
  int keylen = strlen(key);
  // open dir
  if (first_word && key[0] != '.' && key[0] != '/') { // program name
    char *names = getenv("PATH");
    int idx = 0;

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