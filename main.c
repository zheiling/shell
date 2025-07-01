#include "main.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
// #include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void show_invitation();
int res_status = 0;

// TODO: поиск по директориям для работы автозаполнения по Tab
// TODO: не трогать промпт при редактировании строки
// TODO: попробовать считывание по блокам

int main(int argc, char *argv[]) {
  // if (argc < 3) {
  //   fprintf(stderr, "Usage: <dict> <dest>\n");
  //   return 1;
  // }
  // // open files
  // FILE *f_dic = fopen(argv[1], "r");
  // if (f_dic == NULL) {
  //   perror(argv[1]);
  //   return errno;
  // }
  // FILE *f_dest = fopen(argv[2], "w");
  // if (f_dest == NULL) {
  //   perror(argv[2]);
  //   return errno;
  // }

  if (!isatty(0)) {
    fprintf(stderr, "Use only with terminals!\n");
    return 1;
  }

  // analyze vocabulary
  // word_item_t *l_st, *l_cur;
  // word_item_t tmp;

  // int a;
  // char word[MAX_LINE];
  // int i = 0;
  // while ((a = getc(f_dic)) != EOF) {
  //   switch (a) {
  //   case '\n':
  //     l_add(&l_cur, &l_st, word, i);
  //     i = 0;
  //     break;
  //   default:
  //     word[i++] = (char)a;
  //   }
  // }

  struct termios ts1, ts2;

  tcgetattr(0, &ts1);
  memcpy(&ts2, &ts1, sizeof(ts1));
  ts1.c_lflag &= ~(ECHO | ICANON | ISIG);
  ts1.c_cc[VTIME] = 0;
  ts1.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &ts1);

  char buf[MAX_LINE];
  char cs[8];
  int buf_index = 0;
  int read_ret;
  int nw_pos = 0;
  int cur_ind = 0; // cursor indent

  show_invitation();

  while ((read_ret = read(0, cs, 8)) > 0) {
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
      // buf[buf_index] = '\0';
      // int len = p_search_by_key(l_st, buf + nw_pos);
      // if (len == 0)
      //   break;
      // if (len == 1) {
      //   erase_symbols(buf_index);
      //   buf_index = strlen(buf);
      //   buf[buf_index++] = ' ';
      //   nw_pos = buf_index;
      // }
      // write(0, buf, buf_index);
      break;
    case 10:
      buf[buf_index++] = '\n';
      process_line(buf);
      show_invitation();
      buf[0] = '\0';
      buf_index = 0;
      nw_pos = 0;
      break;
    case 23:
      if (buf[buf_index - 1] == ' ') {
        buf_index--;
        nw_pos = last_space(buf, buf_index) + 1;
      }
      erase_symbols(buf_index - nw_pos + 1);
      buf_index = nw_pos;
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

  if (read_ret == -1) {
    return errno;
  }

  return 0;
}

void show_invitation() {
  fflush(stderr);
  printf("%d>", res_status);
  fflush(stdout);
}
