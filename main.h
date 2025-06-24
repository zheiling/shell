typedef struct word_item {
  char *word;
  struct word_item *next;
} word_item_t;

typedef struct op_item {
  char *arg;
  char op[2];
  struct op_item *next;
} op_item;

typedef struct {
  unsigned int par_used : 1; // "" used
  unsigned int inv : 1;      // show invitation
  unsigned int nlin : 1;     // end line
  unsigned int err: 1;
  unsigned int warg: 1;     // treat the next word as an argument
  unsigned int oua: 1;
  unsigned int out: 1;
  unsigned int inp: 1;
  unsigned int bg: 1;
  unsigned int pip: 1;
} flags_t;

enum {
  WORD = 0,   // normal word
  EXT = 1,
  WAND = 2,  // operator: &&
  WPIP = 4,  // operator: |
  WINBC = 8, // operator: &
  WINP = 16,  // operator: <
  WOUA = 32,  // operator: >>
  WOUT = 64,  // operator: >
  WOR = 128,   // operator: ||
};
// list
int l_add(word_item_t **current, word_item_t **start, const char *a, int asize);
int l_shift(word_item_t **src, word_item_t *dst, word_item_t **current);
int convlist(word_item_t *lstart, char ***argv);
// prog
int run_prog(char **argv, flags_t *flags, char rargs[2][255], int in_out[2]);
// pipes
int run_pipes(word_item_t *lstart, flags_t *flags, char rargs[2][255]);
// inpt
int extract_word(char dest[255], flags_t *flags);
void clear_buf();
int analyze_word(char word[255]);
