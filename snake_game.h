#ifndef COOL_SHIT_H
#define COOL_SHIT_H

#define semi_filled_block "\u25A3"
#define triangle_up "\u25B2"
#define triangle_right "\u25B6"
#define triangle_down "\u25BC"
#define triangle_left "\u25C0"
#define diamond "\u25C6"
#define block "\u25A0"
#define INFO_LINES 2
#define TICK_PERIOD_MS 1000
#define TICK_PERIOD_US TICK_PERIOD_MS*1000
#define DEBUG_LINE 1
#define SNAKE_COLOR_PAIR 1
#define FOOD_COLOR_PAIR 2
#define ALERT_COLOR_PAIR 3
#define FRAME_COLOR_PAIR 4
#define INVERSE_COLOR_PAIR 5
#define COLOR_DARK_GREEN 34
#define COLOR_BRIGHT_RED 196
#define COLOR_BRIGHT_YELLOW 226
#define COLOR_BEIGE 7
#define T_DIV 30
#define LOWER_X_LIMIT 1
#define HIGHER_X_LIMIT COLS - 2
#define LOWER_Y_LIMIT INFO_LINES + 1
#define HIGHER_Y_LIMIT LINES - 3


enum direction {up, right, down, left};


struct snake_segment {
	struct list_head list;
	short x;
	short y;
	enum direction heading;
};

struct food {
	short x;
	short y;
};

void snake_construct(struct snake_segment *player, int x, int y, enum direction heading);
void snake_draw(struct snake_segment *player ,struct food *mouse);
void snake_tick(struct snake_segment *player, struct food *mouse);
void snake_set_up_terminal_settings();
void snake_restore_terminal_settings();
void clear_game_screen();
void snake_handle_input(struct snake_segment *player, struct food *mouse);
void snake_change_direction(struct snake_segment *player, int input);
void snake_move_head(struct snake_segment *player);
void print_debug(const char * str);
short snake_next_x_position(struct snake_segment *segment);
short snake_next_y_position(struct snake_segment *segment);
short snake_previous_x_position(struct snake_segment *segment);
short snake_previous_y_position(struct snake_segment *segment);
void kill_everything(struct snake_segment *player, struct food *mouse);
int is_head_colliding(struct snake_segment *player);
void snake_move_body(struct snake_segment *player);
void snake_move_segment(struct snake_segment *segment);
void snake_draw_head(struct snake_segment *player);
void snake_draw_segment(struct snake_segment *segment);
void snake_place_food(struct snake_segment *player, struct food *mouse);
void snake_grow(struct snake_segment *player, struct snake_segment *tail_copy);
void snake_draw_food(struct food *mouse);
void snake_copy_tail(struct snake_segment *player, struct snake_segment *copy);
void snake_initialize_game();
void snake_game_loop();
void *timer_thread_routine();
void *main_thread_routine();
void throw_error(const char *message);
int snake_input_is_acceptable(int input);
void snake_alert_collision(int is_colliding);
int snake_get_size(struct snake_segment *player);
void snake_draw_info(struct snake_segment *player, struct food *mouse);
void snake_pause_game();
void draw_pause_menu(enum menu_commands selected);
void timer_draw_time(int cur_time);
void timer_erase_time();
void timer_pause_time();
#endif
