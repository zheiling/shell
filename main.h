#define MAX_LINE 256

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
  unsigned int nfw: 1; // not first word: needs to reset static index in extract_word
} flags_t;

enum {
  WORD = 0,   // normal word
  EXT = 1,
  WAND = 2,  // operator: &&
  WPIP = 4,  // operator: |
  WINBG = 8, // operator: &
  WINP = 16,  // operator: <
  WOUA = 32,  // operator: >>
  WOUT = 64,  // operator: >
  WOR = 128,   // operator: ||
};

// process line
int process_line(char commline[]);
// list
int l_add(word_item_t **current, word_item_t **start, const char *a, int asize);
int l_shift(word_item_t **src, word_item_t *dst, word_item_t **current);
int convlist(word_item_t *lstart, char ***argv);
int l_search(word_item_t *start, char *key_w, word_item_t **fsstart);
int l_add_2(word_item_t **current, word_item_t **start, char *a);
int l_shift_2(word_item_t **src, char dst[MAX_LINE], word_item_t **current);
int find_occ(word_item_t *start, char *key_w);
// prog
int run_prog(char **argv, flags_t *flags, char rargs[2][MAX_LINE], int in_out[2]);
// pipes
int run_pipes(word_item_t *lstart, flags_t *flags, char rargs[2][MAX_LINE]);
// inpt
int extract_word(char dest[MAX_LINE], flags_t *flags, char line[]);
int analyze_word(char word[MAX_LINE]);
// edit line
int p_search_by_key(char *key, int first_word, word_item_t **wptr);
void move_cursor(int num);
void erase_symbols(int num);
void del_sym(char arr[], int cur_ind, int *buf_index);
void ins_sym(char arr[], int cur_ind, int *buf_index, char a);
int last_space(char arr[], int buf_index);
// helpers
void reset_flags(flags_t *flags);
