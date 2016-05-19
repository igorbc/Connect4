#include <stdio.h>
#include "connect4.h"

int debug = 0;

const int rows = 6;
const int columns = 7;

int level = 0;
int max_level = 6;

int eval[4] = {1, 10, 100, 1000};

int successors(s_state *s, t_player p, s_state *successor);
int utility(s_state s);
int is_terminal(s_state s);

int max_value(s_state s, int *action, int alpha, int beta, int *lv);
int min_value(s_state s, int *action, int alpha, int beta, int *lv);
void copy_state(s_state *dest, s_state *orig);

void update_state(s_state *s);
void update_utility(s_state *s);
void update_four_array(s_state *s);
void update_is_terminal(s_state *s);

int max_value(s_state s, int *action, int alpha, int beta, int *lv){
    s_state successor;

    int v = -MAX;
    int current_max = -MAX;
    int min_action;
    int cur_best_level;

    *lv = level;

    //update_state(&s);

    if (level >= max_level || s.is_terminal){
        return s.utility;
    }

    while(successors(&s, max_player, &successor)){
        level++;
        current_max = min_value(successor, &min_action, alpha, beta, lv);
        level--;
        if(current_max > v || (current_max == v && *lv < cur_best_level)){

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

int min_value(s_state s, int *action, int alpha, int beta, int *lv){
    s_state successor;
    int v = MAX;
    int current_min = MAX;
    int max_action;
    int cur_best_level;

    //update_state(&s);

    *lv = level;
    if (level >= max_level || s.is_terminal){
        return s.utility;
    }

    while(successors(&s, min_player, &successor)){
        level++;
        current_min = max_value(successor, &max_action, alpha, beta, lv);
        level--;
        if(current_min < v || (current_min == v && *lv < cur_best_level)){
            cur_best_level = *lv;

            v = current_min;
            *action = s.action;

            if(debug) printf("new min: %d (action eh %d)\n", v, s.action -1);
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


int successors(s_state *s, t_player p, s_state *successor){
    int i, j;
    if((s->action) == columns) return 0;

//    fprintf(stdout, "level %d successor\n", level);

    for(i = s->action; i < columns; i++){
        for(j = 0; j < rows; j++){
            if(s->s[j][i] == 0){
                s->action = i + 1;
                copy_state(successor, s);

                successor->s[j][i] = p;

                update_state(successor);

                if(debug){
                    printf("\nutility %d\n", successor->utility);
                    //print_gamefield(*successor, level);
                }

                return 1;
            }
        }
    }
    return 0;
}

void update_state(s_state *s){
    s->action = 0;
    update_four_array(s);
    update_utility(s);
}

void update_utility(s_state *s){
    int i, j;
    int four_utility;
    int end_game = 0;

    s->utility = 0;

    for(i = 0; i < 69; i++){
        s->four_count[i][0] = s->four_count[i][1] = s->four_count[i][2] = 0;

        for(j = 0; j < 4; j++){
            switch(s->four_array[i][j]){
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
            else{
                if(s->four_count[i][1] > 0){
                    four_utility = eval[s->four_count[i][1]];
                }
                else if(s->four_count[i][2] > 0){
                    four_utility = -1*eval[s->four_count[i][2]];
                }
                else{
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

    if (end_game){
        s->utility = (end_game * (69000-1));
        if(end_game > 0)
            s->is_terminal = P1_WINS;
        else
            s->is_terminal = P2_WINS;
    }
    else{
        s->is_terminal = DRAW;

        for(i = 0; i < columns; i++){
            if(s->s[rows-1][i]==0){
                s->is_terminal = ACTIVE_GAME;
                break;
            }
        }

        if (s->is_terminal == DRAW){
            s->utility = 0;
        }
    }

//    printf("u: %d\n", s->utility);
}


void update_four_array(s_state *s){
    int fa_count = 0, i, j;
    int sum;
    int (*fa)[4];
    s->sum = 0;
    fa = s->four_array;

    for(i = 0; i < rows; i++){  /* groups of 4 in rows*/
        for(j = 0; j < 4; j++){
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

    for(i = 0; i < columns; i++){ /* groups of 4 in columns*/
        for(j = 0; j < 3; j++){
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
    int diagonal_starting_points_positive[][2] = {{0,3}, {0,2}, {0,1}, {0,0}, {1,0}, {2,0}};
    int diagonal_starting_points_negative[][2] = {{3,0}, {4,0}, {5,0}, {5,1}, {5,2}, {5,3}};
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

void copy_state(s_state *dest, s_state *orig){
    int i, j;
    dest->action = 0;
    //dest->action = orig->action;
    //dest->sum = orig->sum;
    //dest->utility = orig->utility;
    for(i = 0; i < 69; i++){
        for(j = 0; j < 4; j++)
            dest->four_array[i][j] = orig->four_array[i][j];

        dest->four_sum[i] = orig->four_sum[i];
    }

    for(i = 0; i < rows; i++){
        for(j = 0; j < columns; j++)
            dest->s[i][j] = orig->s[i][j];
    }
}
