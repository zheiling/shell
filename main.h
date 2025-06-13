typedef struct word_item {
  char *word;
  struct word_item *next;
} word_item;

typedef struct {
  unsigned int par_open : 1;
  unsigned int compound : 1;
  unsigned int c_par_open : 1;
  unsigned int not_implemented : 1;
  unsigned int reset : 1;
  unsigned int skip_add_w : 1;
} t_flags;

enum {
  INBACK = 01
};

int l_add(word_item **current, word_item **start, const char *a, int asize);
int l_shift(word_item **src, word_item *dst, word_item **current);
