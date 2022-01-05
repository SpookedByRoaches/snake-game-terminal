#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <ncurses.h>
#include <locale.h>
#include <pthread.h>
#include "list.h"
#include "snake_game.h"

pthread_mutex_t *screen_lock;

int main()
{
	screen_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_t main_thread, timer_thread;
	if (pthread_mutex_init(screen_lock, NULL) != 0)
		throw_error("Cannot create pthread_mutex\n");
	pthread_create(&main_thread, NULL, main_thread_routine, (void *) NULL);
	pthread_create(&timer_thread, NULL, timer_thread_routine, (void *)NULL);

	pthread_join(main_thread, NULL);
	pthread_join(timer_thread, NULL);
}

void throw_error(const char *message)
{
	fprintf(stderr, message);
	exit(EXIT_FAILURE);
}

void draw_frame()
{
	pthread_mutex_lock(screen_lock);
	move(INFO_LINES, 0);
	char horz_blanks[COLS];
	memset(horz_blanks, ' ', COLS - 1);
	horz_blanks[COLS - 1] = '\0';
	attron(COLOR_PAIR(FRAME_COLOR_PAIR));
	printw("%s", horz_blanks);
	move(LINES - 1, 0);
	printw("%s", horz_blanks);
	for (int i = INFO_LINES; i < LINES; i++){
		move(i, 0);
		printw(" ");
		move(i, COLS - 1);
		printw(" ");
	}
	attroff(COLOR_PAIR(FRAME_COLOR_PAIR));
	pthread_mutex_unlock(screen_lock);
}

void *timer_thread_routine(void *)
{
	return NULL;
}

void *main_thread_routine(void *)
{
	snake_initialize_game();
}

void snake_initialize_game()
{
	struct snake_segment *player;
	struct food *mouse;
	short init_x, init_y;
	struct timeval cur_t;
	gettimeofday(&cur_t, NULL);
	player = (struct snake_segment*)malloc(sizeof(player));
	mouse = (struct food*)malloc(sizeof(mouse));
	srandom(cur_t.tv_sec);
	snake_set_up_terminal_settings();
	init_x = COLS/2;
	init_y = LINES/2;
	if ((init_x % 2) == 0)
		init_x--;
	snake_construct(player, init_x, init_y, up);
	pthread_mutex_lock(screen_lock);
	if (has_colors() == 0){
		printw("NO COLORS");
	} else {
		printw("HAS COLORS");
	}
	refresh();
	pthread_mutex_unlock(screen_lock);
	draw_frame();
	snake_place_food(player, mouse);
	for (;;){
		snake_tick(player, mouse);
		snake_draw(player, mouse);
	}
	snake_restore_terminal_settings();

}

void snake_construct(struct snake_segment *player, int x, int y, enum direction heading)
{
	player->x = x;
	player->y = y;
	player->heading = heading;
	INIT_LIST_HEAD(&player->list);
}

void snake_tick(struct snake_segment *player, struct food *mouse)
{
	snake_handle_input(player, mouse);
	if (is_head_colliding(player)){
		snake_alert_collision(1);
		return;
	}
	snake_alert_collision(0);
	snake_move_head(player);
	snake_move_body(player);
	if ((player->x == mouse->x) && (player->y == mouse->y)){
		snake_grow_and_replace_food(player, mouse);
	}
}

void snake_alert_collision(int is_colliding)
{
	char *message = "COLLIDING!!";
	short offset = sizeof(message) + 3;
	pthread_mutex_lock(screen_lock);
	if (is_colliding){
		move(0, COLS - offset);
		attron(COLOR_PAIR(ALERT_COLOR_PAIR));
		printw(message);
		attroff(COLOR_PAIR(ALERT_COLOR_PAIR));
	} else {
		move(0, COLS - offset);
		clrtoeol();
	}
	refresh();
	pthread_mutex_unlock(screen_lock);
}

void snake_move_head(struct snake_segment *player)
{
	short nextx = snake_next_x_position(player);
	short nexty = snake_next_y_position(player);
	player->x = nextx;
	player->y = nexty;
}

void snake_grow_and_replace_food(struct snake_segment *player, struct food *mouse)
{
	struct snake_segment *new_segment, *tail_end_segment;
	new_segment = (struct snake_segment *)malloc(sizeof(struct snake_segment));
	tail_end_segment = list_entry(player->list.prev, struct snake_segment, list);
	new_segment->x = snake_previous_x_position(tail_end_segment);
	new_segment->y = snake_previous_y_position(tail_end_segment);
	new_segment->heading = tail_end_segment->heading;
	list_add_tail(&new_segment->list, &player->list);
	snake_place_food(player, mouse);
}

void snake_move_body(struct snake_segment *player)
{
	struct snake_segment *cur_segment, *prev_segment;
	prev_segment = NULL;
	list_for_each_entry_reverse(cur_segment, &player->list, list){
		snake_move_segment(cur_segment);
		if (prev_segment != NULL)
			prev_segment->heading = cur_segment->heading;
		prev_segment = cur_segment;
	}
	if (prev_segment != NULL)
		prev_segment->heading = player->heading;
}

void snake_move_segment(struct snake_segment *segment)
{
	segment->x = snake_next_x_position(segment);
	segment->y = snake_next_y_position(segment);
}

int is_head_colliding(struct snake_segment *player)
{
	struct list_head *i;
	short player_next_x, player_next_y, cur_segment_next_x, cur_segment_next_y;
	struct snake_segment *cur_segment;
	player_next_x = snake_next_x_position(player);
	if ((player_next_x >= COLS - 1) || (player_next_x <= 0))
		return 1;
	player_next_y = snake_next_y_position(player);
	if ((player_next_y >= LINES - 1) || (player_next_y <= INFO_LINES))
		return 1;
	list_for_each(i, &player->list){
		cur_segment = list_entry(i, struct snake_segment, list);
		cur_segment_next_x = snake_next_x_position(cur_segment);
		cur_segment_next_y = snake_next_y_position(cur_segment);
		if ((cur_segment_next_x == player_next_x) && (cur_segment_next_y == player_next_y))
			return 1;
	}

	return 0;
}

short snake_next_x_position(struct snake_segment *segment)
{
	switch (segment->heading){
	case right:
		return segment->x + 2;
	case left:
		return segment->x - 2;
	default:
		return segment->x;
	}
}

short snake_previous_x_position(struct snake_segment *segment)
{
	switch (segment->heading){
	case right:
		return segment->x - 2;
	case left:
		return segment->x + 2;
	default:
		return segment->x;
	}
}

short snake_next_y_position(struct snake_segment *segment)
{
	switch (segment->heading){
	case down:
		return segment->y + 1;
	case up:
		return segment->y - 1;
	default:
		return segment->y;
	}
}

short snake_previous_y_position(struct snake_segment *segment)
{
	switch (segment->heading){
	case down:
		return segment->y - 1;
	case up:
		return segment->y + 1;
	default:
		return segment->y;
	}
}

void snake_draw_food(struct food *mouse)
{
	move(mouse->y, mouse->x);
	attron(COLOR_PAIR(FOOD_COLOR_PAIR));
	printw(diamond);
	attroff(COLOR_PAIR(FOOD_COLOR_PAIR));
}

void snake_draw(struct snake_segment *player, struct food *mouse)
{
	struct timeval before, after;
	struct snake_segment *cur_segment;
	struct list_head *i;
	gettimeofday(&before, NULL);
	pthread_mutex_lock(screen_lock);
	clear_game_screen();
	attron(COLOR_PAIR(SNAKE_COLOR_PAIR));
	snake_draw_head(player);
	list_for_each(i, &player->list){
		cur_segment = list_entry(i, struct snake_segment, list);
		snake_draw_segment(cur_segment);
	}
	snake_draw_food(mouse);
	attroff(COLOR_PAIR(SNAKE_COLOR_PAIR));
	gettimeofday(&after, NULL);
	refresh();
	pthread_mutex_unlock(screen_lock);
}


void snake_draw_head(struct snake_segment *player)
{
	int string_size = 4;
	char direction_char[string_size];

	move(player->y, player->x);
	switch (player->heading){
	case up:
		strncpy(direction_char, triangle_up, string_size);
		break;
	case right:
		strncpy(direction_char, triangle_right, string_size);
		break;
	case down:
		strncpy(direction_char, triangle_down, string_size);
		break;
	case left:
		strncpy(direction_char, triangle_left, string_size);
		break;
	default:
		strncpy(direction_char, "??", string_size);
		break;
	}
	printw("%s", direction_char);
}

void snake_draw_segment(struct snake_segment *segment)
{
	move(segment->y, segment->x);
	printw(block);
}

void clear_game_screen()
{
	char message[COLS - 1];
	memset(message, ' ', COLS - 2);
	message[COLS - 2] = '\0';
	for (int i = INFO_LINES + 1; i < LINES - 1; i++){
		move(i, 1);
		printw("%s", message);
	}
}

void snake_set_up_terminal_settings()
{
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);
	timeout(TICK_PERIOD_MS);
	start_color();
	init_pair(SNAKE_COLOR_PAIR, COLOR_DARK_GREEN, COLOR_BLACK);
	init_pair(ALERT_COLOR_PAIR, COLOR_BLACK, COLOR_BRIGHT_YELLOW);
	init_pair(FOOD_COLOR_PAIR, COLOR_BRIGHT_RED, COLOR_BLACK);
	init_pair(FRAME_COLOR_PAIR, COLOR_BLACK, COLOR_WHITE);
	clear();
}
void snake_restore_terminal_settings()
{
	endwin();
}

void snake_handle_input(struct snake_segment *player, struct food *mouse)
{
	int input;
	flushinp();
	input = getch();

	if ((input == 'q') || (input == 'Q'))
		kill_everything(player, mouse);

	if ((input == KEY_UP) || (input == KEY_DOWN) || (input == KEY_RIGHT) || (input == KEY_LEFT)) 
		snake_change_direction(player, input);
	
}

int snake_input_is_acceptable(int input)
{
	int acceptable_inputs[] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 'q', 'Q'};
	int numel = sizeof(acceptable_inputs)/sizeof(int);
	for (int i = 0; i < numel; i++)
		if (input == acceptable_inputs[i])
			return 1;
	return 0;
}

void snake_change_direction(struct snake_segment *player, int input)
{
	switch (input){
	case KEY_DOWN:
		player->heading = down;
		break;
	case KEY_UP:
		player->heading = up;
		break;
	case KEY_RIGHT:
		player->heading = right;
		break;
	case KEY_LEFT:
		player->heading = left;
		break;
	default:
		player->heading = 6;
	}
}

void kill_everything(struct snake_segment *player, struct food *mouse)
{
	struct list_head *i, *tmp;
	struct snake_segment *cur_segment;
	snake_restore_terminal_settings();
	list_for_each_safe(i, tmp, &player->list){
		cur_segment = list_entry(i, struct snake_segment, list);
		list_del(i);
		free(cur_segment);
	}
	free(player);
	free(mouse);
	free(screen_lock);
	printf("THANKS FOR PLAYING\n");
	exit(0);
}

void snake_place_food(struct snake_segment *player, struct food *mouse)
{
	short x, y, is_ok;
	struct snake_segment *cur_segment;
	is_ok = 0;
	while (!is_ok){
		x = random()%(COLS - 3) + 1;
		if (x % 2 == 0)
			x--;
		y = (random()%(LINES - (INFO_LINES + 2)) + INFO_LINES + 1);
		is_ok = 1;
		if (player->x == x && player->y == y){
			is_ok= 0;
			continue;
		}

		list_for_each_entry(cur_segment, &player->list, list){
			if ((cur_segment->x == x) && (cur_segment->y == y)){
				is_ok = 0;
				break;
			}
		}
	}
	pthread_mutex_lock(screen_lock);
	if (y >= 15){
		move(DEBUG_LINE, 0);
		printw("mouse->y == %d", y);
	}
	pthread_mutex_unlock(screen_lock);
	mouse->x = x;
	mouse->y = y;
}
