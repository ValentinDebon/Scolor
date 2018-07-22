/*
	scolor.c
	Copyright (c) 2018, Valentin Debon

	This file is part of the Scolor game for Linux/macOS/Windows
	subject the BSD 3-Clause License, see LICENSE.txt
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

#define KEYSYM_LEFT	SDLK_LEFT
#define KEYSYM_DOWN	SDLK_DOWN
#define KEYSYM_RIGHT	SDLK_RIGHT

static struct {
	enum {
		GAMEMODE_TITLE = 0,
		GAMEMODE_INGAME = 1,
		GAMEMODE_GAMEOVER = 2,
		GAMEMODE_QUIT = 3
	} gamemode;

	struct {
		void (*keydown)(int);
		void (*pressed)(float, float, int);
		void (*moved)(float, float);
	} handlers[GAMEMODE_QUIT];

	struct {
		SDL_Window *window;
		Uint32 windowID;
		SDL_Renderer *renderer;
		TTF_Font *font;

		Uint32 width, height;
	} display;

	struct {
		bool playHovered;
		struct {
			SDL_Texture *title;
			SDL_Texture *play;
		} textures;
	} title;

	struct {
		struct {
			SDL_Texture *caption;
		} textures;
	} gameover;

	struct {
#define	SCOLOR_YELLOW	0
#define SCOLOR_MAGENTA	1
#define SCOLOR_GREEN	2
#define SCOLOR_NONE	-1
		Uint32 start;
		float step;
		int score;
		int background;
		int current;
		int choice;
	} game;
} scolor;

static void
game_setup(void) {
	scolor.gamemode = GAMEMODE_INGAME;
	scolor.game.start = SDL_GetTicks();
	scolor.game.step = 0;
	scolor.game.score = 0;
	scolor.game.background = rand() % 3;
	scolor.game.current = rand() % 3;
	if(scolor.game.current == scolor.game.background) {
		scolor.game.current += 2;
		scolor.game.current %= 3;
	}
	scolor.game.choice = SCOLOR_NONE;
}

#define MAX(a, b)	((a) > (b) ? (a) : (b))
static void
game_frame(void) {
	Uint32 currentTime = SDL_GetTicks();
	Uint32 roundDuration = MAX(2000 - scolor.game.score * 100, 500);

	if(currentTime - scolor.game.start >= roundDuration) {
		if(scolor.game.choice == scolor.game.current) {
			scolor.game.start = SDL_GetTicks();
			scolor.game.step = 0;
			scolor.game.score += 1;
			scolor.game.background = scolor.game.current;
			scolor.game.current = rand() % 3;
			if(scolor.game.current == scolor.game.background) {
				scolor.game.current += 1;
				scolor.game.current %= 3;
			}
			scolor.game.choice = SCOLOR_NONE;
		} else {
			scolor.gamemode = GAMEMODE_GAMEOVER;
		}
	} else {
		scolor.game.step = (scolor.game.start + roundDuration - currentTime) / (float) roundDuration;
	}

	SDL_Delay(20);
}

static void
display_setup(void) {

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	scolor.display.width = 640;
	scolor.display.height = 480;
	scolor.display.window = SDL_CreateWindow("Scolor",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		scolor.display.width,
		scolor.display.height,
		SDL_WINDOW_RESIZABLE);
	if(scolor.display.window == NULL) {
		fprintf(stderr, "Unable to create window\n");
		exit(EXIT_FAILURE);
	}

	scolor.display.windowID = SDL_GetWindowID(scolor.display.window);

	scolor.display.renderer = SDL_CreateRenderer(scolor.display.window,
		-1, SDL_RENDERER_ACCELERATED);
	if(scolor.display.renderer == NULL) {
		fprintf(stderr, "Unable to create renderer\n");
		exit(EXIT_FAILURE);
	}

	scolor.display.font = TTF_OpenFont("./assets/Montserrat-Light.ttf", 150);
	if(scolor.display.renderer == NULL) {
		fprintf(stderr, "Unable to open font\n");
		exit(EXIT_FAILURE);
	}
}

static void
display_title(void) {
	SDL_Rect rect = {
		.x = 0,
		.y = 0.8 * scolor.display.height,
		.w = 0.34 * scolor.display.width,
		.h = 0.21 * scolor.display.height
	};

	/* Background */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		255, 255, 255, 255);
	SDL_RenderClear(scolor.display.renderer);

	/* Left rectangle */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		255, 255, 0, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Play rectangle */
	rect.x += rect.w;
	SDL_SetRenderDrawColor(scolor.display.renderer,
		255, scolor.title.playHovered ? 120 : 0, 255, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Right rectangle */
	rect.x += rect.w;
	SDL_SetRenderDrawColor(scolor.display.renderer,
		0, 255, 255, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Lazy textures init, never freed */
	/* Title texture */
	if(scolor.title.textures.title == NULL) {
		SDL_Color black = { 0, 0, 0 };
		SDL_Surface *surface = TTF_RenderUTF8_Blended(scolor.display.font,
			"Scølor", black);

		scolor.title.textures.title =
			SDL_CreateTextureFromSurface(scolor.display.renderer, surface);

		SDL_FreeSurface(surface);
	}

	/* Play texture */
	if(scolor.title.textures.play == NULL) {
		SDL_Color white = { 255, 255, 255 };
		SDL_Surface *surface = TTF_RenderUTF8_Blended(scolor.display.font,
			"Play", white);

		scolor.title.textures.play =
			SDL_CreateTextureFromSurface(scolor.display.renderer, surface);

		SDL_FreeSurface(surface);
	}

	/* Title */
	rect.x = 0.333 * scolor.display.width;
	rect.y = 0.25 * scolor.display.height;
	rect.w = 0.333 * scolor.display.width;
	rect.h = 0.2 * scolor.display.height;
	SDL_RenderCopy(scolor.display.renderer, scolor.title.textures.title, NULL, &rect);

	/* Play */
	rect.x = 0.43 * scolor.display.width;
	rect.y = 0.85 * scolor.display.height;
	rect.w = 0.14 * scolor.display.width;
	rect.h = 0.1 * scolor.display.height;
	SDL_RenderCopy(scolor.display.renderer, scolor.title.textures.play, NULL, &rect);

	/* Present */
	SDL_RenderPresent(scolor.display.renderer);
}

static void
display_game(void) {
	SDL_Rect rect = {
		.x = 0,
		.y = scolor.game.step * 0.8 * scolor.display.height,
		.w = scolor.display.width,
		.h = 0.8 * scolor.display.height
	};

	/* Background */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		scolor.game.background == SCOLOR_GREEN ? 0 : 255,
		scolor.game.background == SCOLOR_MAGENTA ? 0 : 255,
		scolor.game.background == SCOLOR_YELLOW ? 0 : 255,
		255);
	SDL_RenderClear(scolor.display.renderer);

	/* Moving rectangle */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		scolor.game.current == SCOLOR_GREEN ? 0 : 255,
		scolor.game.current == SCOLOR_MAGENTA ? 0 : 255,
		scolor.game.current == SCOLOR_YELLOW ? 0 : 255,
		255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Bottom rectangles */
	rect.x = 0;
	rect.y = 0.8 * scolor.display.height;
	rect.w = 0.34 * scolor.display.width;
	rect.h = 0.21 * scolor.display.height;
	/* Left rectangle */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		255, 255, scolor.game.choice == SCOLOR_YELLOW ? 120 : 0, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Play rectangle */
	rect.x += rect.w;
	SDL_SetRenderDrawColor(scolor.display.renderer,
		255, scolor.game.choice == SCOLOR_MAGENTA ? 120 : 0, 255, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Right rectangle */
	rect.x += rect.w;
	SDL_SetRenderDrawColor(scolor.display.renderer,
		scolor.game.choice == SCOLOR_GREEN ? 120 : 0, 255, 255, 255);
	SDL_RenderFillRect(scolor.display.renderer, &rect);

	/* Present */
	SDL_RenderPresent(scolor.display.renderer);
}

static void
display_gameover(void) {
	SDL_Rect rect = {
		.x = 0.1 * scolor.display.width,
		.y = 0.25 * scolor.display.height,
		.w = 0.8 * scolor.display.width,
		.h = 0.2 * scolor.display.height
	};
	SDL_Color red = { 255, 0, 0 };
	char buffer[16];

	/* Background */
	SDL_SetRenderDrawColor(scolor.display.renderer,
		0, 0, 0, 255);
	SDL_RenderClear(scolor.display.renderer);

	/* Caption texture */
	if(scolor.gameover.textures.caption == NULL) {
		SDL_Surface *surface = TTF_RenderUTF8_Blended(scolor.display.font,
			"Game Over, score:", red);

		scolor.gameover.textures.caption =
			SDL_CreateTextureFromSurface(scolor.display.renderer, surface);

		SDL_FreeSurface(surface);
	}

	/* Caption */
	SDL_RenderCopy(scolor.display.renderer, scolor.gameover.textures.caption, NULL, &rect);

	/* Score */
	int length = snprintf(buffer, sizeof(buffer),
		"%d", scolor.game.score);
	SDL_Surface *surface = TTF_RenderUTF8_Blended(scolor.display.font,
		buffer, red);
	SDL_Texture *score =
		SDL_CreateTextureFromSurface(scolor.display.renderer, surface);
	SDL_FreeSurface(surface);
	rect.y += rect.h;
	rect.w = 0.07 * length * scolor.display.width;
	rect.x = (scolor.display.width - rect.w) / 2;
	SDL_RenderCopy(scolor.display.renderer, score, NULL, &rect);

	/* Present */
	SDL_RenderPresent(scolor.display.renderer);
}

static void
display_end(void) {

	TTF_CloseFont(scolor.display.font);

	SDL_DestroyRenderer(scolor.display.renderer);
	SDL_DestroyWindow(scolor.display.window);

	TTF_Quit();
	SDL_Quit();
}

static void
handle_event(bool poll) {
	SDL_Event event;

	do {
		if(poll) {
			SDL_PollEvent(&event);
		} else {
			SDL_WaitEvent(&event);
		}

		switch(event.type) {
		case SDL_QUIT:
			scolor.gamemode = GAMEMODE_QUIT;
			return;
		case SDL_KEYDOWN:
			scolor.handlers[scolor.gamemode]
				.keydown(event.key.keysym.sym);
			return;
		case SDL_MOUSEMOTION:
			scolor.handlers[scolor.gamemode]
				.moved((float) event.motion.x / scolor.display.width,
					(float) event.motion.y / scolor.display.height);
			return;
		case SDL_MOUSEBUTTONDOWN:
			scolor.handlers[scolor.gamemode]
				.pressed((float) event.motion.x / scolor.display.width,
					(float) event.motion.y / scolor.display.height,
					event.button.button);
			return;
		case SDL_WINDOWEVENT:
			if(event.window.windowID == scolor.display.windowID
				&& event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

				scolor.display.width = event.window.data1;
				scolor.display.height = event.window.data2;
				return;
			}
			/* fallthrough */
		default:
			continue;
		}
	} while(!poll);
}

static void
handler_dummy(void) { }

static void
handler_title_pressed(float x,
	float y,
	int button) {

	if(scolor.title.playHovered) {
		scolor.title.playHovered = false;
		game_setup();
	}
}

static void
handler_title_moved(float x,
	float y) {

	if(x >= 0.333 && x <= 0.666 && y >= 0.8) {

		scolor.title.playHovered = true;
	} else {
		scolor.title.playHovered = false;
	}
}

static void
handler_game_keydown(int keysym) {

	switch(keysym) {
	case KEYSYM_LEFT:
		scolor.game.choice = SCOLOR_YELLOW;
		break;
	case KEYSYM_DOWN:
		scolor.game.choice = SCOLOR_MAGENTA;
		break;
	case KEYSYM_RIGHT:
		scolor.game.choice = SCOLOR_GREEN;
		break;
	default:
		break;
	}
}

static void
handler_gameover_pressed(float x,
	float y,
	int button) {

	scolor.gamemode = GAMEMODE_TITLE;
}

int
main(int arg,
	char **argv) {
	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	printf("Scølor game by Valentin Debon - 2018, written with the SDL version %d.%d.%d\n"
		"Linked against SDL version %d.%d.%d\n"
		"Montserrat font by Julieta Ulanovsky\n",
		compiled.major, compiled.minor, compiled.patch,
		linked.major, linked.minor, linked.patch);

	scolor.gamemode = GAMEMODE_TITLE;

	scolor.handlers[GAMEMODE_TITLE].keydown = (void (*)(int))handler_dummy;
	scolor.handlers[GAMEMODE_TITLE].pressed = handler_title_pressed;
	scolor.handlers[GAMEMODE_TITLE].moved = handler_title_moved;

	scolor.handlers[GAMEMODE_INGAME].keydown = handler_game_keydown;
	scolor.handlers[GAMEMODE_INGAME].pressed = (void (*)(float, float, int))handler_dummy;
	scolor.handlers[GAMEMODE_INGAME].moved = (void (*)(float, float))handler_dummy;

	scolor.handlers[GAMEMODE_GAMEOVER].keydown = (void (*)(int))handler_dummy;
	scolor.handlers[GAMEMODE_GAMEOVER].pressed = handler_gameover_pressed;
	scolor.handlers[GAMEMODE_GAMEOVER].moved = (void (*)(float, float))handler_dummy;

	display_setup();
	srand(time(NULL));

	while(scolor.gamemode != GAMEMODE_QUIT) {
		switch(scolor.gamemode) {
		case GAMEMODE_TITLE: {
			display_title();
			handle_event(false);
		} break;
		case GAMEMODE_INGAME: {
			display_game();
			handle_event(true);
			game_frame();
		} break;
		case GAMEMODE_GAMEOVER:
			display_gameover();
			handle_event(false);
			break;
		default:
			break;
		}
	}

	display_end();

	exit(EXIT_SUCCESS);
}

