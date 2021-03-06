#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<sys/time.h>
#include<time.h>
#include "minimax.h"

#define WIN 0
#define MAC 1

int counter;

int timebank, time_per_move, your_botid, game_round, time_left;
char line[1000], player_names[1000], your_bot[1000], field[1000];

s_state state;

int const PLATFORM = MAC;
char value_to_char(int value);
void place_disk(int col, t_player p);

char value_to_char(int value)
{
    if (PLATFORM == WIN)
        return (value == 1) ? 2 : (value == 0) ? 250 : 1;
    else
        return (value == 1) ? 'O' : (value == 0) ? '.' : 'X';

}

void print_gamefield(s_state state, int level)
{
    int i, j, k;

    for (i = get_rows()-1; i >= 0; i--)
    {
        for(k = 0; k < level; k++) printf("   ");
        for (j = 0; j < get_columns(); j++)
        {
            printf(" %c", value_to_char(state.s[i][j]));
        }
        printf("\n");
    }
}

void print_is_terminal(){

    switch(state.is_terminal)
    {
        case DRAW:
            printf("game ended in a draw\n");
            break;

        case ACTIVE_GAME:
            printf("game still going on\n");
            break;

        case P1_WINS:
            printf("player 1 wins!\n");
            break;

        case P2_WINS:
            printf("player 2 wins!\n");
    }
}

void print_four_array(s_state s){
    int i;
    int (*fa)[4];

    update_state(&s);

    fa = s.four_array;

    printf("\nhorizontal\n");
    for(i = 0; i < 69; i++){
        if (i == 24) printf("\nvertical\n");
        if (i == 24+21) printf("\npositive diagonal\n");
        if (i == 24+21+12) printf("\nnegative diagonal\n");

        printf("%c %c %c %c sum: %2d util: %4d (0 = %d, 1 = %d, -1 = %d) \n", value_to_char(fa[i][0]),
                                   value_to_char(fa[i][1]),
                                   value_to_char(fa[i][2]),
                                   value_to_char(fa[i][3]),
                                   s.four_sum[i],
                                   s.four_utility[i],
                                   s.four_count[i][0],
                                   s.four_count[i][1],
                                   s.four_count[i][2]);
    }
    printf("total utility: %d\n", s.utility);
}

void update_gamefield(char field[])
{
    int pos = 0, inc, i, j ;

    for (i = get_rows()-1; i >= 0; i--)
        for (j = 0; j < get_columns(); j++)
        {
            sscanf(&field[pos], "%d%*[,;]%n", &state.s[i][j], &inc);
            pos += inc;
            if (state.s[i][j] == your_botid)
                state.s[i][j] = max_player;
            else if (state.s[i][j] != 0)
                state.s[i][j] = min_player;
        }
}

void print_settings()
{
//*
    struct timeval tval_before, tval_after;
    int i, j;
    float a = 134, b = 32;

    gettimeofday(&tval_before, NULL);

    for(i = 0; i < 100000; i++){
        for(j = 0; j < 10000; j++){
            a = (a / b) + 12;
        }
    }

    gettimeofday(&tval_after, NULL);
    long usec_diff = (long)((tval_after.tv_sec - tval_before.tv_sec)*1000000 + (tval_after.tv_usec - tval_before.tv_usec))/1000;
    printf("time elapsed: %ld\n", usec_diff);

//*/

    printf("settings timebank %d\n", timebank);
    printf("settings time_per_move %d\n", time_per_move);
    printf("settings player_names %s\n", player_names);
    printf("settings your_bot %s\n", your_bot);
    printf("settings your_botid %d\n", your_botid);
}

void play(int mode)
{
    int u = 0, action;
    int p_timebank[3];

    set_level(0);
    t_player p;

    counter = 0;

    p_timebank[max_player] = timebank;
    p_timebank[min_player] = timebank;

    if (mode == 0){
        action = minimax_action(state, max_player, &u, game_round, &timebank);

        printf("place_disc %d\n", action);
        timebank += time_per_move;
    }
    else{
        p = mode;
        action = minimax_action(state, p, &u, game_round, &p_timebank[p]);
        p_timebank[p] += time_per_move;

        printf("col: %d round %d utility: %d\n", action, game_round++, u);
        place_disk(action, p);

        printf("timebank: %d\n",  p_timebank[p]);
        print_is_terminal();

        printf("\n");
    }



}

void initialize_settings()
{
    timebank = 10000;
    time_per_move = 500;
    strcpy(player_names,"player1,player2");
    strcpy(your_bot,"player1");
    your_botid = 1;
}

void place_disk(int col, t_player p)
{
    int i;
    for(i = 0; i < get_rows(); i++){
        if(state.s[i][col] == 0){
            state.s[i][col] = p;

            update_state(&state);
            print_gamefield(state, 0);
            return;
        }
    }
    printf("invalid\n");
}

int main(){
    int col;

    setbuf(stdout, NULL);
    initialize_settings();

	while(1){
        if (gets(line) == NULL) break;
		if (strlen(line) == 0) continue;

        /*printf("line: %s\n", line);*/

		sscanf(line, "settings timebank %d", &timebank);
		sscanf(line, "settings time_per_move %d", &time_per_move);
		sscanf(line, "settings player_names %s", player_names);
		sscanf(line, "settings your_bot %s", your_bot);

		sscanf(line, "settings your_botid %d", &your_botid);
		/* sscanf(line, "settings field_columns %d", &field_columns); 	*/
		/* sscanf(line, "settings field_rows %d", &field_rows); 		*/
		sscanf(line, "update game round %d", &game_round);
		if (sscanf(line, "update game field %s", field)) update_gamefield(field);
		if (sscanf(line, "action move %d", &time_left)) play(0);
        if (strcmp(line, "field")==0) print_gamefield(state, 0);
        if (strcmp(line, "dump")==0) print_settings();
        if (strcmp(line, "terminal")==0) print_is_terminal();
        if (strcmp(line, "toggle ab")==0) toggle_alpha_beta();

        if (sscanf(line, "max %d", &col)) { place_disk(col, max_player); print_is_terminal();}
        if (sscanf(line, "min %d", &col)) { place_disk(col, min_player); print_is_terminal();}
        if (strcmp(line, "move min")==0) play(min_player);
        if (strcmp(line, "move max")==0) play(max_player);

        if (strcmp(line, "print4")==0) print_four_array(state);
	}
	return 0;
}

