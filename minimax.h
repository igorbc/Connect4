#ifndef CONNECT4_H_INCLUDED
#define CONNECT4_H_INCLUDED


#define P1_WINS 1
#define P2_WINS -1
#define DRAW 2
#define ACTIVE_GAME 0
//#define MAX 69000


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
    int is_terminal;
} s_state;


t_player opposite(t_player p);
int minimax_action(s_state s, t_player p, int *utility, int game_round, int *timebank);

int is_better(int first, int second, t_player p);
int close_the_deal(int best_utility, int current_utility, int level, int *shallowest, t_player p);
int same_utility(int best_utility, int current_utility, int level, int *shallowest, int *deepest, t_player p);
int minimax(s_state s, int alpha, int beta, int *lv, t_player p, int *time_up);
int successors(s_state *s, t_player p, s_state *successor);

void update_state(s_state *s);
void update_utility(s_state *s);
void update_four_array(s_state *s);

void toggle_alpha_beta();

void copy_state(s_state *dest, s_state *orig);

int max_value(s_state s, int *action, int alpha, int beta, int *lv);
int min_value(s_state s, int *action, int alpha, int beta, int *lv);
void copy_state(s_state *dest, s_state *orig);

void set_level(int l);

int get_max();
int get_rows();
int get_columns();

#endif // CONNECT4_H_INCLUDED
