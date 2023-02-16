#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdio.h>
#include "SmilerSprites.h"
#include "simplebackgroundmap.h"
#include "simplebackground.h"
#include "TopDown.h"
#include "BlockTile.h"
#include "windowmap.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum NextTileType
{
	FLOOR = 0,
	WALL = 1,
	DOOR = 2,
	KEY = 3
} NextTileType;

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
	NextTileType nextTileType;
} PlayerCharacter;

typedef struct GameManager
{
	BYTE game_is_running;
	BYTE game_is_paused;
	UINT8 inventoryKeys;
	UINT8 lives;
} GameManager;

typedef struct Key
{
	UBYTE got;
	INT16 x;
	INT16 y;
} Key;

PlayerCharacter playerCharacter;
GameManager gameManager;
Key keysOnLevel[4] = {{.got = 0, .x = 18, .y = 1}, {.got = 0, .x = 0, .y = 0}, {.got = 0, .x = 0, .y = 0}, {.got = 0, .x = 0, .y = 0}};

// constants
const unsigned char TILE_MAP[4] = {
		0x00, // floor
		0x26, // wall
		0x27, // door
		0x28	// key
};

const char blankTile[1] = {0x00};
const char keyTile[1] = {0x28};

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

UBYTE debug = 0;

NextTileType checkNextTile(UINT8 newPositionX, UINT8 newPositionY)
{
	UINT16 indexTopLeftX, indexTopLeftY, tileIndexTopLeft;

	indexTopLeftX = (newPositionX - 8) / 8;
	indexTopLeftY = (newPositionY - 16) / 8;
	tileIndexTopLeft = 20 * indexTopLeftY + indexTopLeftX;

	if (TopDown[tileIndexTopLeft] == TILE_MAP[0])
	{
		return FLOOR; // regular floor, walk normally
	}
	else if (TopDown[tileIndexTopLeft] == TILE_MAP[1])
	{
		return WALL; // wall, cannot walk through
	}
	else if (TopDown[tileIndexTopLeft] == TILE_MAP[2])
	{
		return DOOR; // door, cannot walk through, except with key
	}
	else if (TopDown[tileIndexTopLeft] == TILE_MAP[3])
	{
		return KEY; // key, walk through and add to inventory
	}

	return WALL; // wall, cannot walk through
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
	playerCharacter.INTERPOLATION_SPD = 4;

	set_sprite_data(playerCharacter.sprite_id, 2, Smiler);
	set_sprite_tile(0, 0);
	move_sprite(playerCharacter.sprite_id, playerCharacter.x, playerCharacter.y);

	return 0;
}

UINT8 updateKeysInventory(UINT8 value)
{
	gameManager.inventoryKeys = value;

	// update hud
	UINT8 inventory[3] = {0x28, 0x22, gameManager.inventoryKeys + 1};
	set_win_tiles(15, 0, 3, 1, inventory);
	playSound();
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

UBYTE isPlayerCollidingWithKey(PlayerCharacter *player, Key *key)
{

	if (debug == 1)
	{
		printf("%u %u\n", (UINT16)((player->x - 8) / 8), (UINT16)((player->y - 16) / 8));
		printf("%u %u\n\n", (UINT16)(key->x), (UINT16)(key->y));
	}

	if (((player->x - 8) / 8) == key->x && ((player->y - 16) / 8) == key->y)
	{
		return 1;
	}

	return 0;
}

UINT8 drawKeys(void)
{
	UINT8 i;
	for (i = 0; i < 4; i++)
	{
		if (keysOnLevel[i].got == 1)
		{
			set_bkg_tiles(keysOnLevel[i].x, keysOnLevel[i].y, 1, 1, blankTile);
		}
		else if (keysOnLevel[i].x != 0 && keysOnLevel[i].y != 0)
		{
			set_bkg_tiles(keysOnLevel[i].x, keysOnLevel[i].y, 1, 1, keyTile);
			if (isPlayerCollidingWithKey(&playerCharacter, &keysOnLevel[i]) == 1)
			{
				keysOnLevel[i].got = 1;
				updateKeysInventory(gameManager.inventoryKeys + 1);
			}
		}
	}

	return 0;
}

UINT8 init(void)
{
	// font initialization
	font_t min_font;
	font_init();
	min_font = font_load(font_min);
	font_set(min_font);

	// HUD initialization
	set_win_tiles(1, 0, 6, 1, textToTiles("LVL 01"));
	updateKeysInventory(0);
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
			case J_SELECT:
				if (debug == 1)
				{
					debug = 0;
				}
				else
				{
					debug = 1;
				}
				waitpadup();
				break;
			}

			playerCharacter.nextTileType = checkNextTile(playerCharacter.x + playerCharacter.scrolling_x, playerCharacter.y + playerCharacter.scrolling_y);

			if (playerCharacter.nextTileType != WALL)
			{

				// if (gameManager.inventoryKeys <= 0 && playerCharacter.nextTileType == DOOR)
				// {
				// 	playerCharacter.nextTileType = WALL;
				// 	break;
				// }
				// else if (playerCharacter.nextTileType == DOOR)
				// {
				// 	updateKeysInventory(MIN(gameManager.inventoryKeys - 1, 0));
				// 	playerCharacter.nextTileType = WALL;
				// }

				playerCharacter.x += playerCharacter.scrolling_x;
				playerCharacter.y += playerCharacter.scrolling_y;
				interpolateMoveSprite(playerCharacter.sprite_id, playerCharacter.scrolling_x, playerCharacter.scrolling_y, playerCharacter.INTERPOLATION_SPD);
			}
			drawKeys();
		}

		// wait_vbl_done();
		performantDelay(5);
	}

	return 0;
}