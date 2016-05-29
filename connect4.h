#ifndef CONNECT4_H_INCLUDED
#define CONNECT4_H_INCLUDED


#define P1_WINS 1
#define P2_WINS -1
#define DRAW 2
#define ACTIVE_GAME 0
//#define MAX 69000

const int MAX = 69000;

typedef enum {min_player = -1, blank = 0, max_player = 1} t_player;

typedef t_player t_state[6][7];

typedef struct
{
    t_state s;
    int action;
    int utility;
    int four_array[69][4];
    int four_count[69][3];
    int four_sum[69];
    int four_utility[69];
    int sum;
    int is_terminal;
    int level;
} s_state;


int minimax_state(s_state s, t_player p);
t_player opposite(t_player p);
int is_better(int first, int second, t_player p);
int same_utility(int best_utility, int current_utility, int level, int *shallowest, int *deepest, t_player p);
int minimax(s_state s, int *action, int alpha, int beta, int *lv, t_player p);
int successors(s_state *s, t_player p, s_state *successor);


void update_state(s_state *s);
void update_utility(s_state *s);
void update_four_array(s_state *s);

void toggle_alpha_beta();

void copy_state(s_state *dest, s_state *orig);


int max_value(s_state s, int *action, int alpha, int beta, int *lv);
int min_value(s_state s, int *action, int alpha, int beta, int *lv);
void copy_state(s_state *dest, s_state *orig);





#endif // CONNECT4_H_INCLUDED
