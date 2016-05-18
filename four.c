#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define P1_WINS 1
#define P2_WINS -1
#define DRAW 0
#define ACTIVE_GAME 2

#define WIN 0
#define MAC 1

const int rows = 6;
const int columns = 7;

int counter;

int timebank, time_per_move, your_botid, game_round, time_left;
char line[1000], player_names[1000], your_bot[1000], field[1000];
int level = 0;
int max_level = 7;

typedef enum {min_player = -1, blank = 0, max_player = 1} t_player;

typedef t_player t_state[6][7];

typedef struct
{
    t_state s;
    int action;
    int utility;
    int four_array[69][4];
    int four_sum[69];
    int sum;
} s_state;

s_state state;

int const PLATFORM = MAC;
int is_terminal(s_state s);
void compute_utility(s_state *s);
int successors(s_state *s, t_player p, s_state *successor);
int max_value(s_state s, int *action);
int min_value(s_state s, int *action);

char value_to_char(int value)
{
    if (PLATFORM == WIN)
        return (value == 1) ? 2 : (value == 0) ? 250 : 1;
    else
        return (value == 1) ? 'O' : (value == 0) ? '.' : 'X';

}

void print_gamefield(s_state state)
{
    int i, j;
    printf("game:\n");

    for (i = rows-1; i >= 0; i--)
    {
        for (j = 0; j < columns; j++)
        {
            printf(" %c", value_to_char(state.s[i][j]));
        }
        printf("\n");
    }
}

void print_is_terminal(){

    switch(is_terminal(state))
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

void update_four_array(s_state *s)
{
    int fa_count = 0, i, j;
    int sum;
    int (*fa)[4];
    s->sum = 0;

    fa = s->four_array;

    for(i = 0; i < rows; i++) /* groups of 4 in rows*/
    {
        for(j = 0; j < 4; j++)
        {
            sum = 0;
            sum += fa[fa_count][0] = s->s[i][ j ];
            sum += fa[fa_count][1] = s->s[i][j+1];
            sum += fa[fa_count][2] = s->s[i][j+2];
            sum += fa[fa_count][3] = s->s[i][j+3];
            s->four_sum[fa_count] = sum;
            s->sum+= sum;
            fa_count++;
        }

    }
    for(i = 0; i < columns; i++) /* groups of 4 in columns*/
    {
        for(j = 0; j < 3; j++)
        {
            sum = 0;
            sum += fa[fa_count][0] = s->s[ j ][i];
            sum += fa[fa_count][1] = s->s[j+1][i];
            sum += fa[fa_count][2] = s->s[j+2][i];
            sum += fa[fa_count][3] = s->s[j+3][i];
            s->four_sum[fa_count] = sum;
            s->sum+= sum;

            fa_count++;
        }
    }

    int diagonal_groups_to_get[] = {1,2,3,3,2,1};
    int diagonal_starting_points_positive[][2] = {0,3, 0,2, 0,1, 0,0, 1,0, 2,0};
    int diagonal_starting_points_negative[][2] = {3,0, 4,0, 5,0, 5,1, 5,2, 5,3};
    int diagonal_starting_points_counter;
    int diagonal_4_counter;

    int k;

    for(diagonal_starting_points_counter = 0, diagonal_4_counter = 0;
        diagonal_starting_points_counter < 6;
        diagonal_starting_points_counter++,
        diagonal_4_counter++){

        i = diagonal_starting_points_positive[diagonal_starting_points_counter][0];
        j = diagonal_starting_points_positive[diagonal_starting_points_counter][1];
        for(k = 0; k < diagonal_groups_to_get[diagonal_4_counter]; k++, i++, j++){
           sum = 0;
           sum += fa[fa_count][0] = s->s[i][j];
           sum += fa[fa_count][1] = s->s[i+1][j+1];
           sum += fa[fa_count][2] = s->s[i+2][j+2];
           sum += fa[fa_count][3] = s->s[i+3][j+3];
           s->four_sum[fa_count] = sum;
           s->sum+= sum;

           fa_count++;
       }
    }

    for(diagonal_starting_points_counter = 0, diagonal_4_counter = 0;
        diagonal_starting_points_counter < 6;
        diagonal_starting_points_counter++,
        diagonal_4_counter++){

        i = diagonal_starting_points_negative[diagonal_starting_points_counter][0];
        j = diagonal_starting_points_negative[diagonal_starting_points_counter][1];
        for(k = 0; k < diagonal_groups_to_get[diagonal_4_counter]; k++, i--, j++){
           sum = 0;
           sum += fa[fa_count][0] = s->s[i][j];
           sum += fa[fa_count][1] = s->s[i-1][j+1];
           sum += fa[fa_count][2] = s->s[i-2][j+2];
           sum += fa[fa_count][3] = s->s[i-3][j+3];
           s->four_sum[fa_count] = sum;
           s->sum+= sum;

           fa_count++;
       }
    }
}

void print_four_array(s_state s)
{
    int i;
    int (*fa)[4];

    update_four_array(&s);

    fa = s.four_array;

    printf("\nhorizontal\n");
    for(i = 0; i < 24; i++)
    {
        printf("%c %c %c %c sum: %d\n", value_to_char(fa[i][0]),
                                   value_to_char(fa[i][1]),
                                   value_to_char(fa[i][2]),
                                   value_to_char(fa[i][3]),
                                   s.four_sum[i]);
    }
    printf("\nvertical\n");
    for(i = 24; i < 24+21; i++)
    {
        printf("%c %c %c %c sum: %d\n", value_to_char(fa[i][0]),
                                   value_to_char(fa[i][1]),
                                   value_to_char(fa[i][2]),
                                   value_to_char(fa[i][3]),
                                   s.four_sum[i]);
    }

    printf("\npositive diagonal\n");
    for(i = 24+21; i < 24+21+12; i++)
    {
        printf("%c %c %c %c sum: %d\n", value_to_char(fa[i][0]),
                                   value_to_char(fa[i][1]),
                                   value_to_char(fa[i][2]),
                                   value_to_char(fa[i][3]),
                                   s.four_sum[i]);
    }

    printf("\nnegative diagonal\n");
    for(i = 24+21+12; i < 24+21+12+12; i++)
    {
        printf("%c %c %c %c sum: %d\n", value_to_char(fa[i][0]),
                                   value_to_char(fa[i][1]),
                                   value_to_char(fa[i][2]),
                                   value_to_char(fa[i][3]),
                                   s.four_sum[i]);
    }

}

void process_state(s_state *s)
{
    s->action = 0;
    update_four_array(s);
    compute_utility(s);
}

void compute_utility(s_state *s)
{
    int i;
    int four_utility;

    s->utility = 0;

    for(i = 0; i < 69; i++)
    {
        if(s->four_sum[i] < 0)
            four_utility = -1*pow(3,(-1*(s->four_sum[i])*2));
        else if(s->four_sum[i] > 0)
            four_utility = pow(3,(s->four_sum[i]*2));
        else
            four_utility = 0;

        s->utility+=four_utility;
     }
}

int is_terminal(s_state s)
{
    int i;

    for (i = 0; i < 69; i++)
    {
        if(s.four_sum[i] == -4)
            return P2_WINS;
        else if (s.four_sum[i] == 4)
            return P1_WINS;
    }

    for(i = 0; i < columns; i++)
        if(s.s[5][i]==0) return ACTIVE_GAME;

    return DRAW;
}

int utility(s_state s){
    return s.utility;
}

int min_value(s_state s, int *action){
    s_state successor;

    int v = 3*3*3*3*69;
    int current_min = 3*3*3*3*69;

    process_state(&s);
    //fprintf(stdout, "utility of previous state: %d\n", s.utility);
    //fprintf(stdout, "min %d\n", counter++);

    if (level >= max_level)
    {
        return utility(s);
    }

    if(!(is_terminal(s) == ACTIVE_GAME))
    {
        return	utility(s);
    }

    while(successors(&s, min_player, &successor))
    {
        level++;
        current_min = max_value(successor, action);
        level--;
        if(current_min < v)
        {
            v = current_min;
            *action = s.action;
        }
    }

    return	v;
}

int max_value(s_state s, int *action)
{
    s_state successor;

    int v = -3*3*3*3*69;
    int current_max = -3*3*3*3*69;

    process_state(&s);
    /*
    fprintf(stdout, "utility of previous state: %d\n", s.utility);
    */

    //fprintf(stdout, "max %d\n", counter++);

    if (level >= max_level)
    {
        return utility(s);
    }

    if(!(is_terminal(s) == ACTIVE_GAME))
    {
        return utility(s);
    }

    while(successors(&s, max_player, &successor))
    {
        level++;
        current_max = min_value(successor, action);
        level--;
        if(current_max > v)
        {
            v = current_max;
            *action = s.action;
        }

    }
    return	v;
}

void update_gamefield(char field[])
{
    int pos = 0, inc, i, j ;

    for (i = rows-1; i >= 0; i--)
        for (j = 0; j < columns; j++)
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
    printf("settings timebank %d\n", timebank);
    printf("settings time_per_move %d\n", time_per_move);
    printf("settings player_names %s\n", player_names);
    printf("settings your_bot %s\n", your_bot);
    printf("settings your_botid %d\n", your_botid);
}

void play()
{
    level = 0;
    int action;
    counter = 0;
    int u = max_value(state, &action);

    //fprintf(stdout, "utility: %d\n", u);

	printf("place_disc %d\n", action-1);
}

void print_ascii()
{
    int i;
    for(i = 0; i < 256; i++)
    	printf("%c %d\n", (char)i, i);
}

void initialize_settings()
{
    timebank = 10000;
    time_per_move = 500;
    strcpy(player_names,"player1,player2");
    strcpy(your_bot,"player1");
    your_botid = 1;
}

void copy_state(s_state *dest, s_state *orig)
{
    int i, j;
    dest->action = 0;
    //dest->action = orig->action;
    //dest->sum = orig->sum;
    //dest->utility = orig->utility;
    for(i = 0; i < 69; i++)
    {
        for(j = 0; j < 4; j++)
            dest->four_array[i][j] = orig->four_array[i][j];

        dest->four_sum[i] = orig->four_sum[i];
    }

    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < columns; j++)
            dest->s[i][j] = orig->s[i][j];
    }
}

int successors(s_state *s, t_player p, s_state *successor)
{
    int i, j;
    if((s->action) == columns) return 0;

    /*
    fprintf(stdout, "level %d successor\n", level);
    //*/

    for(i = s->action; i < columns; i++)
    {
        for(j = 0; j < rows; j++)
        {
            if(s->s[j][i]==0){
                s->action = i + 1;
                copy_state(successor, s);


                successor->s[j][i] = p;

                /*
                printf("curr action %d: \n", s->action-1);
                print_gamefield(*successor);
                //*/

                return 1;
            }
        }
    }
    return 0;
}

void place_disk(col, p)
{
    int i;
    for(i = 0; i < rows; i++){
        if(state.s[i][col] == 0){
            state.s[i][col] = p;
            print_gamefield(state);
            return;
        }
    }
    printf("invalid");
}

int main()
{
    int row, column;
    int col, p;
    t_player player;

    setbuf(stdout, NULL);

    initialize_settings();

	while(1)
	{
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
		if (sscanf(line, "action move %d", &time_left)) play();
        if (strcmp(line, "field")==0) print_gamefield(state);
        if (strcmp(line, "dump")==0) print_settings();
        if (strcmp(line, "ascii")==0) print_ascii();
        if (strcmp(line, "terminal")==0) print_is_terminal();
        if (strcmp(line, "print4")==0) print_four_array(state);
        if (sscanf(line, "pd %d %d", &col, &p)) place_disk(col, p);
	}
	return 0;
}
