// #include <gb/gb.h>
// #include <gbdk/font.h>
// #include <stdio.h>
// #include "SmilerSprites.h"
// #include "simplebackgroundmap.h"
// #include "simplebackground.h"
// #include "windowmap.h"

// //constants
// INT16 FLOOR_POS_Y = 101;
// INT8 GRAVITY = -2;
// INT8 SPEED_X = 2;
// BYTE game_is_running = 1;

// //global vars TODO: REMOOOOVE FROM HERE PLZ!
// BYTE jumping;
// INT16 current_speed_y;

// void init_sound() {
// 	NR52_REG = 0X80;
// 	NR50_REG = 0X77;
// 	NR51_REG = 0XFF;
// }

// void play_sound() {
// 	NR10_REG = 0x16;
// 	NR11_REG = 0x40;
// 	NR12_REG = 0x73;
// 	NR13_REG = 0x00;
// 	NR14_REG = 0xC3;
// }

// void performant_delay(UINT8 loop_count) {
// 	UINT8 i;
// 	for (i = 0; i < loop_count; i++) {
// 		wait_vbl_done();
// 	}
// }

// INT8 would_hit_surface(INT16 projected_pos_y){
// 	if (projected_pos_y >= FLOOR_POS_Y) {
// 		return projected_pos_y;
// 	}

// 	return -1;
// }

// void player_jump(UINT8 sprite_id, UINT16 sprite_pos[2]) {
// 	INT8 surface_y;

// 	if (!jumping) {
// 		jumping = 1;
// 		current_speed_y = 10;
// 	}

// 	current_speed_y = current_speed_y + GRAVITY;
// 	sprite_pos[1] = sprite_pos[1] - current_speed_y;

// 	surface_y = would_hit_surface(sprite_pos[1]);
// 	if (surface_y != -1) {
// 		jumping = 0;
// 		move_sprite(sprite_id, sprite_pos[0], surface_y);
// 	} else {
// 		move_sprite(sprite_id, sprite_pos[0], sprite_pos[1]);
// 	}
// }

// int main(void){
// 	// font initialization
// 	font_t min_font;
// 	font_init();
// 	min_font = font_load(font_min);
// 	font_set(min_font);

// 	//HUD initialization
// 	set_win_tiles(0, 0, 5, 1, windowmap);
// 	move_win(7, 120);
	
// 	//Sound
// 	init_sound();

// 	//Background
// 	set_bkg_data(37, 7, backgroundtiles);
// 	set_bkg_tiles(0, 0, 40, 18, backgroundmap);

// 	//handling player
// 	INT16 player_pos[2];
// 	player_pos[0] = 10;
// 	player_pos[1] = FLOOR_POS_Y;
// 	jumping = 0;

// 	set_sprite_data(0, 2, Smiler);
// 	set_sprite_tile(0, 0);
// 	move_sprite(0, player_pos[0], player_pos[1]);

// 	SHOW_BKG;
// 	SHOW_SPRITES;
// 	SHOW_WIN;
// 	DISPLAY_ON;

// //UINT8 current_sprite_index = 0;
// //scroll_bkg(1,0);
// 	while(game_is_running) {
// 		if ((joypad() & J_A) || jumping == 1){
// 			if (!jumping) play_sound();
// 			player_jump(0, player_pos);
// 		}

// 		if (joypad() & J_LEFT){
// 			player_pos[0] = player_pos[0] - SPEED_X;
// 			move_sprite(0, player_pos[0], player_pos[1]);
// 		}

// 		if (joypad() & J_RIGHT){
// 			player_pos[0] = player_pos[0] + SPEED_X;
// 			move_sprite(0, player_pos[0], player_pos[1]);
// 		}

// 		if (jumping) {
// 			set_sprite_tile(0, 1);
// 		} else {
// 			set_sprite_tile(0, 0);
// 		}

// 		// scroll_win(0,-1);
// 		performant_delay(5);
// 	}

// 	return 0;
// }