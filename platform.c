/* *****************
 * GameBoy emulator written in C.
 * Credits: Greg Tourville
 ********************/
// v0.04

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// SDL
#include "SDL.h"

// GameBoy
#include "gameboy.h"
#include "sound.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// emulator data
int running = 1;
SDL_Event event;
u8   frameskip = 0;
u8   frames;
u32  f0_ticks;
u32  f1_ticks;
u16  fps;

// color schemes
u32 COLORS_Y[4] = {0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000};
u32 COLORS_R[4] = {0xFFFFFFFF, 0xFFFF9999, 0xFF444499, 0x00000000};
u32 COLORS_G[4] = {0xFFFFFFFF, 0xFF99FF99, 0xFF994444, 0x00000000};
u32 COLORS_B[4] = {0xFFFFFFFF, 0xFF9999FF, 0xFF449944, 0x00000000};
u32 COLORS_O[4] = {0xFFFFFFEE, 0xFFFFFF66, 0xFF444499, 0x00000000};
u32* color_map;

// gameboy color conversion
u32 ColorTo32(u16 cgb)
{
	u8 r = (cgb & 0x001F) << 3;// * 0xFF / 0x1F;
	u8 g = ((cgb >>  5) & 0x001F) << 3;// * 0xFF / 0x1F;
	u8 b = ((cgb >> 10) & 0x001F) << 3;// * 0xFF / 0x1F;

	//cy = (299*r + 587*g + 114*b) / 1000;
	//cb = (-16874*r - 33126*g + 50000*b + 12800000) / 100000;
	//cr = (50000*r - 41869*g - 8131*b + 12800000) / 100000;

	//*v0++ = *v1++ = (cy<<24) | (cb<<16) | (cy<<8) | cr;
	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

// These are the available keys. They correspond to the indices into KEYS.
// They are of type u8 so we can pass them to KeyPress and KeyRelease.
const u8 KeyRight  = 0;
const u8 KeyLeft   = 1;
const u8 KeyUp     = 2;
const u8 KeyDown   = 3;
const u8 KeyA      = 4;
const u8 KeyB      = 5;
const u8 KeySelect = 6;
const u8 KeyStart  = 7;

// key mappings
#define NUM_KEYS 8
u32 KEYS[] = {
	SDLK_RIGHT,
	SDLK_LEFT,
	SDLK_UP,
	SDLK_DOWN,
	SDLK_f,
	SDLK_d,
	SDLK_SPACE,
	SDLK_RETURN,
};

// strings
char  window_caption[100];
char  window_caption_fps[100];
//char  rom_file_buf[260];
//char* rom_file = rom_file_buf;
char  save_file[260];
char  output_file[260];
int   output_file_number = 0;

// pointers
u8*   rom;
u32   rom_size;
u8*   save;
u32   save_size;
FILE* rom_f;
FILE* save_f;

typedef enum {
	screen_unknown,
	screen_opening_credits,
	screen_select_player_count,
	screen_select_game_type,
	screen_select_level,
	screen_in_game
} game_screen;

const char* screen_names[] = {
	"unknown",
	"opening credits",
	"select player count",
	"select game type",
	"select level",
	"in game",
};

#define FINGER_PRINT_3(a, b, c) ((a) | ((b) << 2) | ((c) << 4))
#define FINGER_PRINT_4(a, b, c, d) ((a) | ((b) << 2) | ((c) << 4) | ((d) << 6))

game_screen identify_screen() {
	// screens/finger_print.go says:
	// finger prints for pixels [(127,56) (114,53) (146,18)]:
	// 0_screenshot_opening_credits.png: [0 3 3]
	// 1_screenshot_player_count_select.png: [2 3 0]
	// 2_screenshot_game_type_select.png: [1 1 1]
	// 3_screenshot_level_select.png: [1 0 1]
	// 4_screenshot_in_game.png: [0 3 0]

	u8 finger_print = FINGER_PRINT_3(
		gb_fb[56][127] & 3,
		gb_fb[53][114] & 3,
		gb_fb[18][146] & 3
	);

	if(finger_print == FINGER_PRINT_3(0, 3, 3))
		return screen_opening_credits;
	if(finger_print == FINGER_PRINT_3(2, 3, 0))
		return screen_select_player_count;
	if(finger_print == FINGER_PRINT_3(1, 1, 1))
		return screen_select_game_type;
	if(finger_print == FINGER_PRINT_3(1, 0, 1))
		return screen_select_level;
	if(finger_print == FINGER_PRINT_3(0, 3, 0))
		return screen_in_game;
	return screen_unknown;
}

typedef enum {
	tile_unknown,
	tile_I,
	tile_Z,
	tile_S,
	tile_L,
	tile_J,
	tile_O,
	tile_T,
	tile_empty,
	tile_flicker,
	tile_brick,
	tile_pause,
	tile_game_over
} tetromino;

const char* tetromino_letters = "?IZSLJOT./=PX";

tetromino identify_tetromino(int x, int y) {
	// tetrominos/finger_print.go says:
	// finger prints for pixels [(2,1) (2,2) (6,2) (3,3)]:
	// i0.png: [2 1 1 1]
	// i1.png: [2 1 2 1]
	// i2.png: [1 2 1 1]
	// i3.png: [1 2 2 1]
	// i4.png: [1 1 2 1]
	// i5.png: [1 1 1 2]
	// z.png: [1 1 1 3]
	// s.png: [2 3 2 0]
	// l.png: [2 2 2 2]
	// j.png: [1 3 1 0]
	// o.png: [0 3 0 3]
	// t.png: [1 0 1 1]
	// empty.png: [0 0 0 0]
	// flicker.png: [1 1 1 1]
	// brick.png: [0 1 2 1]
	// pause.png: [0 0 3 3]
	// game_over.png: [3 0 1 1]

	u8 finger_print = FINGER_PRINT_4(
		gb_fb[y + 1][x + 2] & 3,
		gb_fb[y + 2][x + 2] & 3,
		gb_fb[y + 2][x + 6] & 3,
		gb_fb[y + 3][x + 3] & 3
	);

	if(finger_print == FINGER_PRINT_4(2, 1, 1, 1))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(2, 1, 2, 1))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(1, 2, 1, 1))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(1, 2, 2, 1))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(1, 1, 2, 1))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(1, 1, 1, 2))
		return tile_I;
	if(finger_print == FINGER_PRINT_4(1, 1, 1, 3))
		return tile_Z;
	if(finger_print == FINGER_PRINT_4(2, 3, 2, 0))
		return tile_S;
	if(finger_print == FINGER_PRINT_4(2, 2, 2, 2))
		return tile_L;
	if(finger_print == FINGER_PRINT_4(1, 3, 1, 0))
		return tile_J;
	if(finger_print == FINGER_PRINT_4(0, 3, 0, 3))
		return tile_O;
	if(finger_print == FINGER_PRINT_4(1, 0, 1, 1))
		return tile_T;
	if(finger_print == FINGER_PRINT_4(0, 0, 0, 0))
		return tile_empty;
	if(finger_print == FINGER_PRINT_4(1, 1, 1, 1))
		return tile_flicker;
	if(finger_print == FINGER_PRINT_4(0, 1, 2, 1))
		return tile_brick;
	if(finger_print == FINGER_PRINT_4(0, 0, 3, 3))
		return tile_pause;
	if(finger_print == FINGER_PRINT_4(3, 0, 1, 1))
		return tile_game_over;

	return tile_unknown;
}

// current_screen remembers the screen that was last identified.
// Start current_screen with something invalid so that the very first time we
// compare it, we get a change.
game_screen current_screen = 999;

// last_screen remembers the last screen shown to the player. We use it to
// detect when the screen is "in a steady state", i.e. when two consecutive
// frames are identical. There is visual tearing when switching e.g. between
// the menu and the game state.
u8 last_screen[LCD_HEIGHT][LCD_WIDTH];

// The Tetris board is 10 by 18 tiles in size.
const int board_width = 10;
const int board_height = 18;

int main(int argc, char **argv)
{
	int     i, x, y;
	int     scale = 1;
	u8      j;
	u32     romread;
	u32     old_ticks;
	u32     new_ticks;
	int     delay;
	u32*    s;
	int     quit_seq;
	u32		fb[LCD_HEIGHT][LCD_WIDTH];
	char	*rom_file = NULL;
	SDL_Window *win = NULL;
	SDL_Renderer *ren = NULL;
	SDL_Texture *tex = NULL;

	FILE* recording_file = fopen("recording", "wb");

	int c;

	// Start last_screen with something invalid. This way the very first
	// comparison to the current screen is a change.
	last_screen[0][0] = 0xFF;

	while((c = getopt(argc, argv, "hm:f:")) != -1)
	{
		switch (c) {
			case 'h':
				printf("Usage: %s [-m magnification (1..9)] -f GB_ROM\n", argv[0]);
				return 0;
			case 'f':
				rom_file = optarg;
				break;
			case 'm':
				if('1' <= *optarg && *optarg <= '9')
					scale = *optarg - '0';
				break;
			default:
				printf("?? getopt returned character code 0%o ??\n", c);
				return -1;
		}
	}

	if(rom_file == NULL)
	{
		printf("Please specify a file to load.\nUse -h to see help.\n");
		return -1;
	}

	// Load ROM file
	if((access(rom_file, F_OK) != -1) &&
			((rom_f = fopen(rom_file, "rb")) != NULL))
	{
		printf("%s: Opening %s.\n", __func__, rom_file);
		rom_f = fopen(rom_file, "rb");
	}
	else
	{
		printf("%s: File \"%s\" not found.\n", __func__, rom_file);
		return -1;
	}

	// Init SDL
#if _SOUND_H
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#else
	SDL_Init(SDL_INIT_VIDEO);
#endif
	SDL_CreateWindowAndRenderer(scale * LCD_WIDTH, scale * LCD_HEIGHT, SDL_WINDOW_RESIZABLE,
		&win, &ren);
	SDL_assert(win != NULL);
	SDL_assert(ren != NULL);
	tex = SDL_CreateTexture(ren,
				    SDL_PIXELFORMAT_RGBA32,
				    SDL_TEXTUREACCESS_STREAMING,
				    LCD_WIDTH, LCD_HEIGHT);
	SDL_assert(tex != NULL);

	// Start Audio
#if _SOUND_H
	SDLAudioStart();
#endif // _SOUND_H

	fseek(rom_f, 0, SEEK_END);
	rom_size = ftell(rom_f);
	rewind(rom_f);
	rom = (u8*)malloc(rom_size);
	for (i = 0; i < rom_size; i++)
		rom[i] = 0xFF;

	romread = fread(rom, sizeof(u8), rom_size, rom_f);
	fclose(rom_f);

	// Load SAVE file (if it exists)
	sprintf(save_file, "%s.sav", rom_file);
	save_size = GetSaveSize(rom);
	save = (u8*)malloc(save_size);
	save_f = fopen(save_file, "rb");
	if (save_f)
	{
		fseek(save_f, 0, SEEK_SET);
		fread(save, sizeof(u8), save_size, save_f);
		fclose(save_f);
	}

	// Start the emulator
	LoadROM(rom, rom_size, save, save_size);

	color_map = COLORS_Y;

	new_ticks= SDL_GetTicks();
	f1_ticks = new_ticks;
	quit_seq = 0;
	while (running && quit_seq != 3)
	{
		// handle input / events
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = 0;
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_0)
				{
					frameskip = 0;
					SetFrameSkip(0);
				}
				else if (event.key.keysym.sym == SDLK_1)
				{
					frameskip = 1;
					SetFrameSkip(0);
				}
				else if (event.key.keysym.sym == SDLK_2)
				{
					frameskip = 2;
					SetFrameSkip(2);
				}
				else if (event.key.keysym.sym == SDLK_3)
				{
					frameskip = 3;
					SetFrameSkip(3);
				}
				else if (event.key.keysym.sym == SDLK_9)
				{
					frameskip = 9;
					SetFrameSkip(10);
				}
				else if (event.key.keysym.sym == SDLK_r)
					color_map = COLORS_R;
				else if (event.key.keysym.sym == SDLK_g)
					color_map = COLORS_G;
				else if (event.key.keysym.sym == SDLK_b)
					color_map = COLORS_B;
				else if (event.key.keysym.sym == SDLK_o)
					color_map = COLORS_O;
				else if (event.key.keysym.sym == SDLK_y)
					color_map = COLORS_Y;
				else if (event.key.keysym.sym == SDLK_q)
					quit_seq |= 1;
				else if (event.key.keysym.sym == SDLK_LCTRL)
					quit_seq |= 2;
				else
					for (j = 0; j < NUM_KEYS; j++) {
						if (KEYS[j] == event.key.keysym.sym) {
							KeyPress(j);
							break;
						}
					}
			}
			else if(event.type == SDL_KEYUP)
			{
				for (j = 0; j < NUM_KEYS; j++) {
					if (KEYS[j] == event.key.keysym.sym)
					{
						KeyRelease(j);
						break;
					}
				}
				if (event.key.keysym.sym == SDLK_q
						|| event.key.keysym.sym == SDLK_LCTRL)
				{
					quit_seq = 0;
				}
			}
			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1) {
				for (y = 0; y < LCD_HEIGHT; y++)
					for (x = 0; x < LCD_WIDTH; x++)
						gb_fb[y][x] = gb_fb[y][x] | (gb_fb[y][x] << 2) | (gb_fb[y][x] << 4) | (gb_fb[y][x] << 6);
				sprintf(output_file, "screenshot_%03d.png", output_file_number++);
				stbi_write_png(output_file, LCD_WIDTH, LCD_HEIGHT, 1, gb_fb, LCD_WIDTH);
			}
			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F2) {
				printf("game screen: %s\n", screen_names[identify_screen()]);
				for (y = 0; y < board_height; y++) {
					for (x = 0; x < board_width; x++) {
						// The board starts at the top of the screen, 2 tiles
						// from the left. That is why 2 is added to x.
						tetromino t = identify_tetromino((2 + x) * 8, y * 8);
						char letter = tetromino_letters[t];
						printf("%c", letter);
					}
					printf("\n");
				}
			}
		}

		old_ticks = SDL_GetTicks();
		usleep(16 - (new_ticks - old_ticks));

		// emulate frame
		RunFrame();

		u8 key_states[NUM_KEYS] = {
			IsKeyDown(KeyRight),
			IsKeyDown(KeyLeft),
			IsKeyDown(KeyUp),
			IsKeyDown(KeyDown),
			IsKeyDown(KeyA),
			IsKeyDown(KeyB),
			IsKeyDown(KeySelect),
			IsKeyDown(KeyStart),
		};
		fwrite(gb_fb, 1, LCD_WIDTH * LCD_HEIGHT, recording_file);
		fwrite(key_states, 1, NUM_KEYS, recording_file);

		int skip = (memcmp(last_screen, gb_fb, LCD_WIDTH * LCD_HEIGHT) != 0);
		memcpy(last_screen, gb_fb, LCD_WIDTH * LCD_HEIGHT);
		if(!skip) {
			game_screen last_screen = current_screen;
			current_screen = identify_screen();
			if(last_screen != current_screen) {
				printf("game screen: %s\n", screen_names[current_screen]);
			}
		}

		if (gb_framecount == 0)
		{
			// convert colors
			if (cgb_enable)
				for (y = 0; y < LCD_HEIGHT; y++)
					for (x = 0; x < LCD_WIDTH; x++)
						fb[y][x] = ColorTo32(cgb_fb[y][x]);
			else
				for (y = 0; y < LCD_HEIGHT; y++)
					for (x = 0; x < LCD_WIDTH; x++)
						fb[y][x] = color_map[gb_fb[y][x] & 3];

			// render
			SDL_UpdateTexture(tex, NULL, fb, LCD_WIDTH * sizeof(uint32_t));
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_RenderPresent(ren);

			//old_ticks = new_ticks;
			new_ticks = SDL_GetTicks();
			frames++;
			if (frames % 0x80 == 0)
			{
				f0_ticks = f1_ticks;
				f1_ticks = new_ticks;
				fps = (128*1000)/(f1_ticks - f0_ticks) * (gb_frameskip ? gb_frameskip : 1);
				sprintf(window_caption_fps, "%s - %u fps", window_caption, fps);
				SDL_SetWindowTitle(win, window_caption_fps);
			}

			// Cap at 60FPS unless using frameskip
			if (!frameskip)
			{
				delay = 16 - (new_ticks - old_ticks);
				SDL_Delay(delay > 0 ? delay : 0);
			}
		}
	}

	// Save game before exit
	if (save_size)        {
		save_f = fopen(save_file, "wb");
		if (save_f)
		{
			fseek(save_f, 0, SEEK_SET);
			fwrite(save, 1, save_size, save_f);
			fclose(save_f);
		}
	}

	// Clean up
	free(rom);
	free(save);
	SDL_Quit();

	fclose(recording_file);

	return 0;
}

