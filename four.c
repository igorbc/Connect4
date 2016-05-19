#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define P1_WINS 1
#define P2_WINS -1
#define DRAW 0
#define ACTIVE_GAME 2
#define MAX 69000;

#define WIN 0
#define MAC 1

const int rows = 6;
const int columns = 7;

int counter;
int debug = 0;

int timebank, time_per_move, your_botid, game_round, time_left;
char line[1000], player_names[1000], your_bot[1000], field[1000];
int level = 0;
int max_level = 6;

int eval[4] = {1, 10, 100, 1000};

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
} s_state;

s_state state;
s_state chosen_state;

int const PLATFORM = MAC;
int is_terminal(s_state s);
void compute_utility(s_state *s);
int successors(s_state *s, t_player p, s_state *successor);
int max_value(s_state s, int *action, int alpha, int beta, int *cs);
int min_value(s_state s, int *action, int alpha, int beta, int *cs);
void place_disk(int col, t_player p);
void copy_state(s_state *dest, s_state *orig);
void process_state(s_state *s);

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

    for (i = rows-1; i >= 0; i--)
    {
        for(k = 0; k < level; k++) printf("   ");
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

    process_state(&s);

    fa = s.four_array;

    printf("\nhorizontal\n");
    for(i = 0; i < 69; i++)
    {
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

void process_state(s_state *s)
{
    s->action = 0;
    update_four_array(s);
    compute_utility(s);
}

void compute_utility(s_state *s)
{
    int i, j;
    int four_utility;
    int end_game = 0;

    s->utility = 0;

    for(i = 0; i < 69; i++)
    {
        s->four_count[i][0] = s->four_count[i][1] = s->four_count[i][2] = 0;

        for(j = 0; j < 4; j++)
        {
            switch(s->four_array[i][j])
            {
                case 0:
                s->four_count[i][0]++;
                break;

                case max_player:
                s->four_count[i][1]++;
                break;

                case min_player:
                s->four_count[i][2]++;
                break;
            }

            if(s->four_count[i][1] > 0 && s->four_count[i][2] > 0 )
                four_utility = 0;
            else
            {
                if(s->four_count[i][1] > 0)
                {
                    four_utility = eval[s->four_count[i][1]];
                }
                else if(s->four_count[i][2] > 0)
                {
                    four_utility = -1*eval[s->four_count[i][2]];
                }
                else
                {
                    four_utility = 0;
                }
            }
        }

        if(s->four_sum[i] == 4){
            end_game = 1;
        }
        else if(s->four_sum[i] == -4){
            end_game = -1;
        }

        s->four_utility[i] = four_utility;
        s->utility+=four_utility;
     }
    if (end_game)
    {
        s->utility = (end_game * (69000-1));
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

int min_value(s_state s, int *action, int alpha, int beta, int *lv){
    s_state successor;
    int v = MAX;
    int current_min = MAX;
    int max_action;
    int cur_best_level;

    *lv = level;
    if (level >= max_level)
    {
        process_state(&s);
        return utility(s);
    }

    process_state(&s);

    int terminal_status = is_terminal(s);
    if(terminal_status != ACTIVE_GAME)
    {
        if (terminal_status == DRAW)
        {
            return 0;
        }
        else
        {
            return utility(s);
        }
    }

    while(successors(&s, min_player, &successor))
    {
        level++;
        current_min = max_value(successor, &max_action, alpha, beta, lv);
        level--;
        if(current_min < v || (current_min == v && *lv < cur_best_level))
        {
            cur_best_level = *lv;

            v = current_min;
            *action = s.action;

            if(debug)
                printf("new min: %d (action eh %d)\n", v, s.action -1);
            //copy_state(cs, &successor);

            //printf("\nutility %d\n", v);
            //print_gamefield(successor, level);

       //     if (v <= alpha)
       //         return v;
        }
       // beta = (v < beta)? v: beta;
    }

    return	v;
}

int max_value(s_state s, int *action, int alpha, int beta, int *lv)
{
    s_state successor;

    int v = -MAX;
    int current_max = -MAX;
    int min_action;
    int cur_best_level;

    *lv = level;

    if (level >= max_level)
    {
        process_state(&s);
        return utility(s);
    }

    process_state(&s);
    int terminal_status = is_terminal(s);
    if(terminal_status != ACTIVE_GAME)
    {
        if (terminal_status == DRAW)
        {
            return 0;
        }
        else
        {
            return utility(s);
        }
    }

    while(successors(&s, max_player, &successor))
    {
        level++;
        current_max = min_value(successor, &min_action, alpha, beta, lv);
        level--;
        if(current_max > v || (current_max == v && *lv < cur_best_level))
        {

            //if( (current_max == v && *lv < cur_best_level) && level == 0)
            //    printf("level anterior: %d, novo: %d (valor %d)\n",cur_best_level, *lv, current_max);

            cur_best_level = *lv;
            v = current_max;
            *action = s.action;

            //if(level == 0)
            //    printf("cur best level: %d\n", cur_best_level);


            //copy_state(cs, &successor);
            //printf("\nutility %d\n", v);
            //print_gamefield(successor, level);

            if (debug)
                printf("new max: %d (action eh %d)\n", v, s.action -1);

            //if (v >= beta)
            //    return v;
        }
        //alpha = (v > alpha)? v: alpha;
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

void play(int mode)
{
    int u, action, alpha, beta;
    level = 0;

    counter = 0;
    alpha = -MAX;
    beta = MAX;

    int lv;

    switch(mode)
    {
        case 0:
        u = max_value(state, &action, alpha, beta, &lv);
        //fprintf(stdout, "utility: %d\n", u);
        printf("place_disc %d\n", action-1);
        break;

        case max_player:
        u = max_value(state, &action, alpha, beta, &lv);
        printf("col: %d utility: %d\n", action-1, u);
        place_disk(action-1, max_player);
        print_is_terminal();
        break;

        case min_player:
        u = min_value(state, &action, alpha, beta, &lv);
        printf("col: %d utility: %d\n", action-1, u);
        place_disk(action-1, min_player);
        print_is_terminal();
        break;

        default:
        printf("invalid play mode\n");
    }
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
            if(s->s[j][i] == 0){
                s->action = i + 1;
                copy_state(successor, s);

                successor->s[j][i] = p;

                if(debug)
                {
                    process_state(successor);
                    printf("\nutility %d\n", utility(*successor));
                    print_gamefield(*successor, level);
                }

                return 1;
            }
        }
    }
    return 0;
}

void place_disk(int col, t_player p)
{
    int i;
    for(i = 0; i < rows; i++){
        if(state.s[i][col] == 0){
            state.s[i][col] = p;
            print_gamefield(state, 0);
            process_state(&state);
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
		if (sscanf(line, "action move %d", &time_left)) play(0);
        if (strcmp(line, "field")==0) print_gamefield(state, 0);
        if (strcmp(line, "dump")==0) print_settings();
        if (strcmp(line, "ascii")==0) print_ascii();
        if (strcmp(line, "terminal")==0) print_is_terminal();

        if (sscanf(line, "max %d", &col)) { place_disk(col, max_player); print_is_terminal();}
        if (sscanf(line, "min %d", &col)) { place_disk(col, min_player); print_is_terminal();}
        if (strcmp(line, "move min")==0) play(min_player);
        if (strcmp(line, "move max")==0) play(max_player);

        if (strcmp(line, "print4")==0)
        {
            print_four_array(state);
        }
        if (strcmp(line, "cs")==0)
        {
            print_gamefield(chosen_state, 0);
        }
	}
	return 0;
}
