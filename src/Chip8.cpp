#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <time.h>


#include "Chip8.h"

using namespace std;

unsigned char fontset[80] = {
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {}

void Chip8::initialize() {
	srand(time(0));             // Seed the RNG

	opcode = 0;                 // Opcode
	pc = 0x200;                 // Program counter starts at 0x200 (512)
	I = 0;                      // Index register
	sp = 0;                     // Stack pointer

	memset(&memory, 0, 4096);   // RAM
	memset(&stack, 0, 16);      // Stack
	memset(&V, 0, 16);          // Registers
	memset(&gfx, 0, 64 * 32);     // Graphics (64*32 = number of pixels)
	memset(&key, 0, 16);        // Key states

	delay_timer = 0;            // Delay timer
	sound_timer = 0;            // Sound timer

	for (int i = 0; i < 80; i++) { // Load font set into memory (0x00-0x50)
		memory[i] = fontset[i];
	}
}

bool Chip8::load(const char* filename) {
	initialize();                   // initialize registers, memory

	FILE* rom = fopen(filename, "rb");

	if (rom == NULL)                // failed to open file, so failed to load
		return false;

	// Get the size of the file
	fseek(rom, 0, SEEK_END);
	long file_size = ftell(rom);
	fseek(rom, 0, SEEK_SET);

	if (file_size > 4096 - 512)     // if file can't fit into memory, failed to load
		return false;

	// Read from the file into a buffer
	uint8_t* buffer = new uint8_t[file_size];
	fread(buffer, sizeof(uint8_t), file_size, rom);

	// Copy from buffer into memory starting at 0x200 (512)
	for (int i = 0; i < file_size; i++) {
		memory[i + 0x200] = buffer[i];
	}
	cout << "LOADED " << filename << endl;

	fclose(rom);
	delete[] buffer;
	buffer = NULL;

	return true;
}

void Chip8::emulateCycle() {
	opcode = (memory[pc] << 8) | memory[pc + 1];        // grab the opcode at location pc

	uint16_t X = (opcode & 0x0F00) >> 8;
	uint16_t Y = (opcode & 0x00F0) >> 4;
	uint8_t N = opcode & 0x000F;
	uint8_t NN = opcode & 0x00FF;
	uint16_t NNN = opcode & 0x0FFF;

	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode & 0x000F) {
				case 0x0:
					for (int i = 0; i < 64 * 32; i++) {
						gfx[i] = 0;
					}
					drawFlag = true;
					pc += 2;
					break;
				case 0xE:
					sp--;
					pc = stack[sp];
					pc += 2;
					break;
			}
			break;
		case 0x1000:
			pc = NNN;
			break;
		case 0x2000:
			stack[sp] = pc;
			sp++;
			pc = NNN;
			break;
		case 0x3000:
			if (V[X] == NN)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x4000:
			if (V[X] != NN)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x5000:
			if (V[X] == V[Y])
				pc += 4;
			else
				pc += 2;
			break;
		case 0x6000:
			V[X] = NN;
			pc += 2;
			break;
		case 0x7000:
			V[X] += NN;
			pc += 2;
			break;
		case 0x8000:
			switch (opcode & 0x000F) {
				case 0x0:
					V[X] = V[Y];
					pc += 2;
					break;
				case 0x1:
					V[X] |= V[Y];
					pc += 2;
					break;
				case 0x2:
					V[X] &= V[Y];
					pc += 2;
					break;
				case 0x3:
					V[X] ^= V[Y];
					pc += 2;
					break;
				case 0x4:
					if (V[X] > 255 - V[Y])
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[X] += V[Y];
					pc += 2;
					break;
				case 0x5:
					if (V[X] < V[Y])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[X] -= V[Y];
					pc += 2;
					break;
				case 0x6:			// for some reason the wikipedia page is wrong for this case and a few others
					V[0xF] = V[X] & 1;
					V[X] >>= 1;
					pc += 2;
					break;
				case 0x7:
					if (V[Y] < V[X])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[X] = V[Y] - V[X];
					pc += 2;
					break;
				case 0xE:
					V[0xF] = V[X] >> 7;
					V[X] <<= 1;
					pc += 2;
					break;
			}
			break;
		case 0x9000:
			if (V[X] != V[Y])
				pc += 4;
			else 
				pc += 2;
			break;
		case 0xA000:
			I = NNN;
			pc += 2;
			break;
		case 0xB000:
			pc = V[0] + NNN;
			break;
		case 0xC000:
			V[X] = (rand() % 256) & NN;
			pc += 2;
			break;
		case 0xD000: // honestly had no idea how to implemenet this, so this is what the tutorial says
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
							V[0xF] = 1;
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc += 2;
			break;
		}
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x9E:
					if (key[V[X]] != 0)
						pc += 4;
					else
						pc += 2;
					break;
				case 0xA1:
					if (key[V[X]] == 0)
						pc += 4;
					else
						pc += 2;
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x07:
					V[X] = delay_timer;
					pc += 2;
					break;
				case 0x0A:
				{
					bool keyPressed = false;

					for (int i = 0; i < 16; i++) {
						if (key[i] != 0) {
							keyPressed = true;
							V[X] = i;
						}
					}
					if (!keyPressed) {
						return;
					}
					pc += 2;
					break;
				}
				case 0x15:
					if (I + V[X] > 0xFFF) // overflow
						V[0xF] = 1;
					else
						V[0xF] = 0;
					delay_timer = V[X];
					pc += 2;
					break;
				case 0x18:
					sound_timer = V[X];
					pc += 2;
					break;
				case 0x1E:
					I += V[X];
					pc += 2;
					break;
				case 0x29:
					I = V[X] * 5;							// 5 per row, so the value times 5 is the first in its respective row
					pc += 2;
					break;
				case 0x33:
				{
					memory[I] = V[X] / 100;					// hundreds place
					memory[I + 1] = (V[X] / 10) % 10;		// tens place
					memory[I + 2] = V[X] % 10;				// units
					pc += 2;
					break;
				}
				case 0x55:
					for (int v = 0; v <= X; v++) {
						memory[I] = V[v];					// increment I each time instead of using i since we have to increase I each time too
						I++;
					}

					pc += 2;
					break;
				case 0x65:
					for (int v = 0; v <= X; v++) {
						V[v] = memory[I];
						I++;
					}

					pc += 2;
					break;
				}
			break;
	}

	// Update timers
	if (delay_timer > 0)
		delay_timer--;
	if (sound_timer > 0) // no sound implemented
		sound_timer--;
}
