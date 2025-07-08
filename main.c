#include "main.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void show_invitation();
int res_status = 0;

// TODO: использовать l_shift_2 (вместо временного node просто подставлять адрес
// строки);

int main(int argc, char *argv[]) {
  if (!isatty(0)) {
    fprintf(stderr, "Use only with terminals!\n");
    return 1;
  }

  struct termios ts1, ts2;

  tcgetattr(0, &ts1);
  memcpy(&ts2, &ts1, sizeof(ts1));
  ts1.c_lflag &= ~(ECHO | ICANON | ISIG);
  ts1.c_cc[VTIME] = 0;
  ts1.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &ts1);

  char buf[MAX_LINE];
  word_item_t *arg_list = NULL;
  flags_t flags;
  char rargs[2][MAX_LINE]; // <, (> || >>)
  word_item_t tmp;

  while (1) {
    reset_flags(&flags);
    show_invitation();
    res_status = 0;
    if (edit_line(buf)) {
      perror("Get line");
    } else {
      process_line(buf, &arg_list, &flags, rargs);
      
      if (flags.err) {
        // TODO: здесь централизованно выводить ошибки
        while (!l_shift(&arg_list, &tmp, NULL))
          ;
        continue;
      }

      if (flags.pip) {
        res_status = run_pipes(arg_list, &flags, rargs);
      } else {
        int in_out[2];
        in_out[0] = -1;
        in_out[1] = -1;
        char **argvs;
        convlist(arg_list, &argvs);
        res_status = run_prog(argvs, &flags, rargs, in_out);
      }
      if (WIFEXITED(res_status)) {
        res_status = WEXITSTATUS(res_status);
      } else {
        res_status = WTERMSIG(res_status);
      }
    }
    errno = 0;
    buf[0] = '\0';
  }

  return 0;
}