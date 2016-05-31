#include <stdio.h>
#include <sys/time.h>
#include "minimax.h"

int debug = 0;

int level = 0;
int max_level = 7;

int eval[4] = {1, 10, 100, 1000};

int ab = 1;

const int MAX = 6900;
const int rows = 6;
const int columns = 7;
long int time_to_spend;

t_player opposite(t_player p){
    if(p == max_player)
        return min_player;
    return max_player;
}

long get_time_diff(struct timeval tval_before, struct timeval tval_after){
    return (long)((tval_after.tv_sec - tval_before.tv_sec)*1000000 + (tval_after.tv_usec - tval_before.tv_usec))/1000;
}

int minimax_action(s_state s, t_player p, int *utility, int game_round, int *timebank){
    s_state best;
    int lv;
    int best_utility;
    int alpha, beta;
    int current_utility, action;
    int shallowest;
    int time_up;

    struct timeval tval_before, tval_after;

    max_level = 5;

    action = 2;

    if(game_round > 2){
        time_to_spend = (*timebank) < 1500? *timebank: 1500;
    }
    else {
        time_to_spend = 800;
    }

    *timebank -= time_to_spend;
    while(time_to_spend > 0){
        gettimeofday(&tval_before, NULL);

        s.action = 0;

        best_utility = (int) opposite(p) * MAX;;
        alpha = -get_max();
        beta = get_max();
        shallowest = max_level + 1;

        //if(game_round > 20) max_level = 10;
        time_up = 0;
        while(successors(&s, p, &best)){
            current_utility = minimax(best, alpha, beta, &lv, opposite(p), &time_up);
            if(!time_up){
                if(is_better(current_utility, best_utility, p) || close_the_deal(best_utility, current_utility, lv, &shallowest, p)){
                    best_utility = current_utility;
                    action = s.action - 1;
                }
            }
        }
        *utility = best_utility;
        max_level++;

        gettimeofday(&tval_after, NULL);
        time_to_spend-= get_time_diff(tval_before, tval_after);

     //   printf("current level: %d time_to_spend: %ld\n", max_level, time_to_spend);
    }
    //printf("max level: %d time_to_spend: %ld\n", max_level, time_to_spend);
    *timebank += time_to_spend;
    return action;
}

int is_better(int first, int second, t_player p){
    if(p == max_player){
        return (first > second);
    }
    else{
        return (first < second);
    }
}

int close_the_deal(int best_utility, int current_utility, int level, int *shallowest, t_player p){
    if(best_utility != current_utility) return 0;

    if(level < *shallowest){
        if (*shallowest != max_level + 1 && debug) printf(" winning. getting the shallowest (%d instead of %d)\n", level, *shallowest);
		*shallowest = level;
		return 1;
	}

	return 0;
}

// not used
int same_utility(int best_utility, int current_utility, int level, int *shallowest, int *deepest, t_player p){

    if(best_utility != current_utility) return 0;

	// is losing, get the deepest
	if((p == max_player && best_utility <= 0) || (p == min_player && best_utility >= 0)){
		if(*deepest != -1){
            if(level < max_level && debug) printf(" losing. getting the deepest (%d instead of %d)\n", level, *deepest);
			*deepest = level;
			return 1;
		}
	}
	// is winning, get the shallowest
	else{
		if(level < *shallowest){
            if (*shallowest != max_level + 1 && debug) printf(" winning. getting the shallowest (%d instead of %d)\n", level, *shallowest);
			*shallowest = level;
			return 1;
		}
	}
	return 0;
}

int minimax(s_state s, int alpha, int beta, int *lv, t_player p, int *time_up){
    s_state successor;
    struct timeval tval_before, tval_after;

    gettimeofday(&tval_before, NULL);

    int v = (int) opposite(p) * MAX;
    int current_best = v;

    *lv = level;

    if(time_to_spend <= 0){
        *time_up = 1;
        return v;
    }

    if (level >= max_level || s.is_terminal){
        gettimeofday(&tval_after, NULL);
        time_to_spend-= get_time_diff(tval_before, tval_after);
        return s.utility;
    }

    while(successors(&s, p, &successor)){
        level++;
        current_best = minimax(successor, alpha, beta, lv, opposite(p), time_up);
        level--;
        if(is_better(current_best, v, p)){

            v = current_best;

            if (debug) printf("new max: %d (action eh %d)\n", v, s.action -1);

            if (ab){
                if(p == max_player){
                    if (v >= beta) {
                        gettimeofday(&tval_after, NULL);
                        time_to_spend-= get_time_diff(tval_before, tval_after);
                        return v;
                    }
                }
                else{
                    if (v <= alpha){
                        gettimeofday(&tval_after, NULL);
                        time_to_spend-= get_time_diff(tval_before, tval_after);
                        return v;
                    }
                }
            }
        }
        if (ab){
            if(p == max_player)
                alpha = (v > alpha)? v: alpha;
            else
                beta = (v < beta)? v: beta;
        }
    }
    gettimeofday(&tval_after, NULL);
    time_to_spend-= get_time_diff(tval_before, tval_after);
    return v;
}

int successors(s_state *s, t_player p, s_state *successor){
    int i, j;
    if((s->action) == columns) return 0;

    for(i = s->action; i < columns; i++){
        for(j = 0; j < rows; j++){
            if(s->s[j][i] == 0){
                s->action = i + 1;
                copy_state(successor, s);

                successor->s[j][i] = p;

                update_state(successor);

                if(debug){
                    printf("\nutility %d\n", successor->utility);
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
        s->utility = (end_game * (MAX-1));
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
}

void update_four_array(s_state *s){
    int fa_count = 0, i, j;
    int sum;
    int (*fa)[4];

    fa = s->four_array;

    for(i = 0; i < rows; i++){  /* groups of 4 in rows*/
        for(j = 0; j < 4; j++){
            sum = 0;
            sum += fa[fa_count][0] = s->s[i][ j ];
            sum += fa[fa_count][1] = s->s[i][j+1];
            sum += fa[fa_count][2] = s->s[i][j+2];
            sum += fa[fa_count][3] = s->s[i][j+3];
            s->four_sum[fa_count] = sum;
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

           fa_count++;
       }
    }
}

void toggle_alpha_beta(){
    ab = !ab;
    if(ab) printf("alpha beta true\n");
    else printf("alpha beta false\n");
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


int max_value(s_state s, int *action, int alpha, int beta, int *lv){
    s_state successor;

    int v = -MAX;
    int current_max = -MAX;
    int min_action;
    int cur_best_level;

    *lv = level;

    if (level >= max_level || s.is_terminal){
        return s.utility;
    }

    while(successors(&s, max_player, &successor)){
        level++;
        current_max = min_value(successor, &min_action, alpha, beta, lv);
        level--;
        if(current_max > v || (current_max == v && *lv < cur_best_level)){

            v = current_max;
            *action = s.action;

            cur_best_level = *lv;

            if (debug)
                printf("new max: %d (action eh %d)\n", v, s.action -1);

            if (ab)
                if (v >= beta){
                    return v;
                }
        }
        if (ab)
            alpha = (v > alpha)? v: alpha;
    }
    return	v;
}

int min_value(s_state s, int *action, int alpha, int beta, int *lv){
    s_state successor;
    int v = MAX;
    int current_min = MAX;
    int max_action;
    int cur_best_level;

    *lv = level;
    if (level >= max_level || s.is_terminal){
        return s.utility;
    }

    while(successors(&s, min_player, &successor)){
        level++;
        current_min = max_value(successor, &max_action, alpha, beta, lv);
        level--;
        if(current_min < v || (current_min == v && *lv < cur_best_level)){

            v = current_min;
            *action = s.action;

            cur_best_level = *lv;

            if(debug) printf("new min: %d (action eh %d)\n", v, s.action -1);

            if(ab)
                if (v <= alpha)
                    return v;
        }
        if(ab)
            beta = (v < beta)? v: beta;
    }
    return	v;
}


void set_level(int l){
    level = l;
}

int get_max(){
    return MAX;
}
int get_rows(){
    return rows;
}
int get_columns(){
    return columns;
}
