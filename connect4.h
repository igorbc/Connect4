#ifndef CONNECT4_H_INCLUDED
#define CONNECT4_H_INCLUDED
#endif // CONNECT4_H_INCLUDED

#define P1_WINS 1
#define P2_WINS -1
#define DRAW 2
#define ACTIVE_GAME 0
#define MAX 69000;

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
} s_state;
