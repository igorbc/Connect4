#include<stdio.h>
#include<string.h>

const int rows = 6;
const int columns = 7;

int timebank, time_per_move, your_botid, game_round, time_left;
char line[1000], player_names[1000], your_bot[1000], field[1000];

typedef enum {min_player = -1, blank = 0, max_player = 1} t_player;

typedef t_player t_state[6][7];

t_state state;

void print()
{
	for (int i = rows-1; i >= 0; i--)
	{
		for (int j = 0; j < columns; j++)
		{
			char c = state[i][j] == 1 ? '*' : '#';
			c = state[i][j] == 0 ? '.' : c;
			printf("%c ", c);
		}
		printf("\n");
	}
}

void update_gamefield(char field[])
{
	int pos = 0, inc;

	for (int i = rows-1; i >= 0; i--)
		for (int j = 0; j < columns; j++)
		{
			sscanf(&field[pos], "%d%*[,;]%n", &state[i][j], &inc);
			pos += inc;
			if (state[i][j] == your_botid)
				state[i][j] = max_player;
			else if (state[i][j] != 0)
				state[i][j] = min_player;
		}
}

void play()
{
	printf("place_disc %d\n", 0);
}

int main()
{
	int row, column;
	t_player player;

	setbuf(stdout, NULL);
	while (1)
	{
		if (gets(line) == NULL) break;
		if (strlen(line) == 0) continue;
		sscanf(line, "settings timebank %d", &timebank);
		sscanf(line, "settings time_per_move %d", &time_per_move);
		sscanf(line, "settings player_names %s", player_names);
		sscanf(line, "settings your_bot %s", your_bot);
		sscanf(line, "settings your_botid %d", &your_botid);
		// sscanf(line, "settings field_columns %d", &field_columns); 	// unsupported
		// sscanf(line, "settings field_rows %d", &field_rows); 		// unsupported
		sscanf(line, "update game round %d", &game_round);
		if (sscanf(line, "update game field %s", field)) update_gamefield(field);
		if (sscanf(line, "action move %d", &time_left)) play();
	}
	return 0;
}