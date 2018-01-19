// Credit to JamesGriffin on github for his implementation for the keymap array since I wasn't sure how to implement the keypad with SDL
// also to all the ROMs included in this project, because I couldn't find them anywhere else

#include <iostream>
#include <chrono>
#include <thread>
#include "SDL.h"
#include "Chip8.h"

using namespace std;

uint8_t keymap[16] = {
SDLK_x,
SDLK_1,
SDLK_2,
SDLK_3,
SDLK_q,
SDLK_w,
SDLK_e,
SDLK_a,
SDLK_s,
SDLK_d,
SDLK_z,
SDLK_c,
SDLK_4,
SDLK_r,
SDLK_f,
SDLK_v,
};

int main(int argc, char **argv) {
	cout << "READING FILE" << endl;
	if (argc != 2) {
		cout << "Invalid arguments" << endl;
		return 1;
	}
	
	Chip8 chip8 = Chip8();
	if (!chip8.load(argv[1])) {
		cout << "Failed to load file " << argv[1] << ".\n";
		return 1;
	}

	int width = 1024;
	int height = 512;

	uint32_t pixels[64 * 32];

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("CHIP8 Emulator - by Kameron Chehraz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, width, height);
	SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	while (true) {
		chip8.emulateCycle();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) return 0; // won't be able to quit the program without this

			// Update key states
			if (event.type == SDL_KEYDOWN) {
				for (int i = 0; i < 16; i++) {
					if (event.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 1; // update key state to pressed
					}
				}
			}
			if (event.type == SDL_KEYUP) {
				for (int i = 0; i < 16; i++) {
					if (event.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 0; // update key state to not pressed
					}
				}
			}
		}
		
		// Check if we need to redraw the screen
		if (chip8.drawFlag) {
			// Fill pixel buffer
			for (int i = 0; i < 64 * 32; i++) {
				pixels[i] = (0x00FFFFFF * chip8.gfx[i]) | 0xFF000000; // black and white pixels (0 for black, 1 for white)
			}

			SDL_UpdateTexture(screenTexture, NULL, pixels, 64 * sizeof(uint32_t)); // update screen texture
			SDL_RenderCopy(renderer, screenTexture, NULL, NULL); // fill screen with texture
			SDL_RenderPresent(renderer); // render to screen

			// Reset drawFlag
			chip8.drawFlag = false;
		}

		// Delay to run the game slower
		this_thread::sleep_for(std::chrono::microseconds(1200));
	}
	return 0;
}
