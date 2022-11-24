#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdio.h>
#include "SmilerSprites.h"
#include "simplebackgroundmap.h"
#include "simplebackground.h"
#include "TopDown.h"
#include "BlockTile.h"
#include "windowmap.h"

typedef struct PlayerCharacter
{
	UINT8 sprite_id;
	UINT8 SPEED_X;
	UINT8 SPEED_Y;
	UINT8 INTERPOLATION_SPD;
	INT16 x;
	INT16 y;
	INT8 scrolling_x;
	INT8 scrolling_y;
} PlayerCharacter;

typedef struct GameManager
{
	BYTE game_is_running;
	BYTE game_is_paused;
	UINT8 inv_keys;
	UINT8 lives;
} GameManager;

PlayerCharacter playerCharacter;
GameManager gameManager;

// constants
// TODO: move this to a constant
const unsigned char BLANK_MAP[1] = {0x00};

UINT8 initSound(void)
{
	NR52_REG = 0X80;
	NR50_REG = 0X77;
	NR51_REG = 0XFF;

	return 0;
}

UINT8 playSound(void)
{
	NR10_REG = 0x16;
	NR11_REG = 0x40;
	NR12_REG = 0x73;
	NR13_REG = 0x00;
	NR14_REG = 0xC3;

	return 0;
}

UINT8 performantDelay(UINT8 loop_count)
{
	UINT8 i;
	for (i = 0; i < loop_count; i++)
	{
		wait_vbl_done();
	}

	return 0;
}

UBYTE canPlayerMove(UINT8 new_pos_x, UINT8 new_pos_y)
{

	UINT16 index_top_left_x, index_top_left_y, tile_index_top_left;
	UBYTE can_move;

	index_top_left_x = (new_pos_x - 8) / 8;
	index_top_left_y = (new_pos_y - 16) / 8;
	tile_index_top_left = 20 * index_top_left_y + index_top_left_x;

	can_move = TopDown[tile_index_top_left] == BLANK_MAP[0];

	return can_move;
}

UINT8 interpolateMoveSprite(UINT8 sprite_id, INT8 move_x, INT8 move_y, UINT8 speed)
{
	while (move_x != 0)
	{
		scroll_sprite(sprite_id, move_x < 0 ? -speed : speed, 0);
		// move_x += sign(move_x)
		move_x += (move_x < 0 ? speed : -speed);
		wait_vbl_done();
	}

	while (move_y != 0)
	{
		scroll_sprite(sprite_id, 0, move_y < 0 ? -speed : speed);
		// move_y += sign(move_y)
		move_y += (move_y < 0 ? speed : -speed);
		wait_vbl_done();
	}

	return 0;
}

UINT8 initPlayer(void)
{
	playerCharacter.sprite_id = 0;
	playerCharacter.x = 16;
	playerCharacter.y = 24;
	playerCharacter.SPEED_X = 8;
	playerCharacter.SPEED_Y = 8;
	playerCharacter.INTERPOLATION_SPD = 2;

	set_sprite_data(playerCharacter.sprite_id, 2, Smiler);
	set_sprite_tile(0, 0);
	move_sprite(playerCharacter.sprite_id, playerCharacter.x, playerCharacter.y);

	return 0;
}

unsigned char *textToTiles(char *ch)
{

	unsigned char r[10];
	UINT8 i;

	for (i = 0; i < 10; ++i)
	{
		switch (ch[i])
		{
		case '0':
			r[i] = 0x01;
			break;
		case '1':
			r[i] = 0x02;
			break;
		case '2':
			r[i] = 0x03;
			break;
		case '3':
			r[i] = 0x04;
			break;
		case '4':
			r[i] = 0x05;
			break;
		case '5':
			r[i] = 0x06;
			break;
		case '6':
			r[i] = 0x07;
			break;
		case '7':
			r[i] = 0x08;
			break;
		case '8':
			r[i] = 0x09;
			break;
		case '9':
			r[i] = 0x0A;
			break;
		default:
			if (ch[i] - 54 > 36 || ch[i] - 54 < 0)
			{
				r[i] = 0x00;
			}
			else
			{
				r[i] = ch[i] - 54;
			}
			break;
		}
	}

	unsigned char *tiled_text = (unsigned char *)r;
	return tiled_text;
}

UINT8 init(void)
{

	// font initialization
	font_t min_font;
	font_init();
	min_font = font_load(font_min);
	font_set(min_font);

	gameManager.inv_keys = 0;

	// HUD initialization
	set_win_tiles(1, 0, 6, 1, textToTiles("LVL 01"));
	UINT8 inventory[3] = {0x28, 0x22, gameManager.inv_keys + 3};
	set_win_tiles(15, 0, 3, 1, inventory);
	move_win(7, 136);

	// Sound
	initSound();

	initPlayer();

	// Background
	set_bkg_data(37, 4, BlockTile);
	set_bkg_tiles(0, 0, 20, 18, TopDown);

	SHOW_BKG;
	SHOW_SPRITES;
	SHOW_WIN;
	DISPLAY_ON;

	return 0;
}

UINT8 main(void)
{
	init();
	gameManager.game_is_running = 1;

	while (gameManager.game_is_running)
	{
		if (gameManager.game_is_paused)
		{
			switch (joypad())
			{
			case J_START:
				waitpadup();
				set_win_tiles(1, 0, 6, 1, textToTiles("LVL 01"));
				gameManager.game_is_paused = 0;
				break;
			}
		}
		else
		{
			playerCharacter.scrolling_x = 0;
			playerCharacter.scrolling_y = 0;
			switch (joypad())
			{
			case J_START:
				waitpadup();
				gameManager.game_is_paused = 1;
				set_win_tiles(1, 0, 6, 1, textToTiles("PAUSED"));
				break;
			case J_LEFT:
				playerCharacter.scrolling_x = playerCharacter.SPEED_X * -1;
				break;
			case J_RIGHT:
				playerCharacter.scrolling_x = playerCharacter.SPEED_X;
				break;
			case J_UP:
				playerCharacter.scrolling_y = playerCharacter.SPEED_Y * -1;
				break;
			case J_DOWN:
				playerCharacter.scrolling_y = playerCharacter.SPEED_Y;
				break;
			}

			if (canPlayerMove(playerCharacter.x + playerCharacter.scrolling_x, playerCharacter.y + playerCharacter.scrolling_y))
			{
				playerCharacter.x += playerCharacter.scrolling_x;
				playerCharacter.y += playerCharacter.scrolling_y;
				interpolateMoveSprite(playerCharacter.sprite_id, playerCharacter.scrolling_x, playerCharacter.scrolling_y, playerCharacter.INTERPOLATION_SPD);
			}
			delay(100);
		}

		wait_vbl_done();
		// performantDelay(6);
	}

	return 0;
}