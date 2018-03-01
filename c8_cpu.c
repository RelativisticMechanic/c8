#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include <SDL/SDL.h>

#include "c8_cpu.h"

c8 cpu;

#define c8_incr(x) (cpu.ip += 2)

static uint8_t c8_font[80] =
{
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

static int c8_keymap[0x10] = {
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f
};

void c8_exit(int code) {
	SDL_Quit();
	exit(code); // exit with error code
}
void c8_err(const char* fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	c8_exit(-1);
}
// helper function that just sets the processor state to 0
void c8_zero(void) {
	cpu.i = 0; cpu.dt = 0; cpu.st = 0; cpu.sp = 0;
	cpu.ip = 0;
	// set registers to 0
	for(int i = 0; i < 16; i++)
		cpu.v[i] = 0;
	// set graphics memory to 0
	for(int i = 0; i < C8_W*C8_H; i++)
		cpu.px[i] = 0;
	// set memory to 0
	for(int i = 0; i < C8_MEM; i++)
		cpu.m[i] = 0;
	// set stack to 0
	for(int i = 0; i < 256; i++)
		cpu.stk[i] = 0;
	// set inputs to 0
	for(int i = 0; i < 16; i++)
		cpu.inp[i] = 0;
}
// draw routine
void c8_draw(void) {
	SDL_Surface* drawsurf = SDL_GetVideoSurface();
	SDL_LockSurface(drawsurf); // lock surface for pixel manipulation
	uint32_t* px = (uint32_t*)drawsurf->pixels;
	memset(px, 0, drawsurf->w * drawsurf->h * sizeof(uint32_t));
	
	for(int i = 0; i < SCREEN_H; i++) {
		for(int j = 0; j < SCREEN_W; j++) {
			px[j + i * drawsurf->w] = cpu.px[(j/10) + (i/10)*C8_W] ? 0xffffffff : 0xf9912fff;
		}
	}
	
	SDL_UnlockSurface(drawsurf);
	SDL_Flip(drawsurf);
	SDL_Delay(15);
}
// beep function
void c8_beep(void) {
	// todo : implement sound output
}
// updates our timers at 60 hz
void c8_timers(void) {
	if(cpu.dt > 0)
		cpu.dt--;
	if(cpu.st > 0)
		cpu.st--;
	if(cpu.st != 0)
		c8_beep();
}
// after 1 cycle call this
void c8_refresh(SDL_Event* ev) {
	uint8_t* keys = SDL_GetKeyState(NULL);
	if(keys[SDLK_ESCAPE]) 
	{
		c8_err("escape called. quitting.\n");
	}
	if(keys[SDLK_p])
	{
		while(true)
		{
			if(SDL_PollEvent(ev))
			{
				if(keys[SDLK_p])
					break;
				else if(keys[SDLK_ESCAPE])
					c8_err("escaped called. quitting.\n");
			}
		}
	}
}
void c8_emulate_instruction(void) {
	uint8_t* keys;
	for(int cyc = 0; cyc < 10; cyc++) 
	{
		uint16_t opcode = (cpu.m[cpu.ip] << 8) | cpu.m[cpu.ip+1];
		switch(opcode & 0xF000)
		{
			case 0x0000:
				switch(opcode & 0x000F)
				{
					case 0x0000:
						// 00E0 - Clear screen
						memset(&cpu.px[0], 0, C8_W*C8_H);
						c8_incr();
						break;
					case 0x000E:
						// 000E - return from call
						cpu.ip = cpu.stk[--cpu.sp] + 2;
						break;
					default:
						c8_err("invalid instruction in binary: 0x%x\n", cpu.ip - C8_LOAD_POINT);
						break;
				}
				break;
			case 0x1000:
				// 1NNN - Jump to NNN
				cpu.ip = opcode & 0x0FFF;
				break;
			case 0x2000:
				// 2NNN - Call NNN
				cpu.stk[cpu.sp++] = cpu.ip;
				cpu.ip = opcode & 0x0FFF;
				break;
			case 0x3000:
				// 3XNN if rX == NN, skip next instruction
				if(cpu.v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				{
					c8_incr();
				}
				c8_incr();
				break;
			case 0x4000:
				// 4XNN if rX != NN, skip next instruction
				if(cpu.v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				{
					c8_incr();
				}
				c8_incr();
				break;
			case 0x5000:
				// 5XY0 if rX == rY, skip next instruction
				if(cpu.v[(opcode & 0x0F00) >> 8] == cpu.v[(opcode & 0x00F0) >> 4]) 
				{
					c8_incr();
				}
				c8_incr();
				break;
			case 0x6000:
				// 6XNN rX = NN
				cpu.v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
				c8_incr();
				break;
			case 0x7000:
				// 7XNN rX += NN
				cpu.v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
				c8_incr();
				break;
			case 0x8000:
				{
					uint8_t vx = cpu.v[(opcode & 0x0F00) >> 8];
					uint8_t vy = cpu.v[(opcode & 0x00F0) >> 4];
					uint8_t vx_idx = (opcode & 0x0F00) >> 8;
					uint8_t vy_idx = (opcode & 0x00F0) >> 4;
					switch(opcode & 0x000F)
					{
						case 0x0000:
							// 8XY1 - VX = VY
							cpu.v[vx_idx] = vy;
							c8_incr();
							break;
						case 0x0001:
							cpu.v[vx_idx] = vx | vy;
							c8_incr();
							break;
						case 0x0002:
							cpu.v[vx_idx] = vx & vy;
							c8_incr();
							break;
						case 0x0003:
							cpu.v[vx_idx] = vx ^ vy;
							c8_incr();
							break;
						case 0x0004:
							// Add vy to vx, if overflow, then set VF to 1
							if(((int)vx + (int)vy) < 256)
								cpu.v[15] &= 0;
							else
								cpu.v[15] = 1;
							cpu.v[vx_idx] = vx + vy;
							c8_incr();
							break;
						case 0x0005:
							if(((int)vx - (int)vy) >= 0)
								cpu.v[15] = 1;
							else
								cpu.v[15] &= 0;
							cpu.v[vx_idx] = vx - vy;
							c8_incr();
							break;
						case 0x0006:
							// 8X06
							// shr vx by 1, vf is set to the value of the lsb
							// before shift
							cpu.v[15] = vx & 7;
							cpu.v[vx_idx] = vx >> 1;
							c8_incr();
							break;
						case 0x0007:
							// 8XY7 vx = vx - vy
							if((int)vx - (int)vy > 0)
								cpu.v[15] = 1;
							else
								cpu.v[15] &= 0;
						
							cpu.v[vx_idx] = vy - vx;
							c8_incr();
							break;
						case 0x000E:
							// 8XYE - shl vx by 1, vf is set to msb before shift
							cpu.v[15] = vx >> 7;
							cpu.v[vx_idx] = vx << 1;
							c8_incr();
							break;
						default:
							c8_err("invalid instruction in binary: 0x%x\n", cpu.ip - C8_LOAD_POINT);
							break;
					}
				}
				break;
			case 0x9000:
				// 5XY0 if rX != rY, skip next instruction
				if(cpu.v[(opcode & 0x0F00) >> 8] != cpu.v[(opcode & 0x00F0) >> 4]) 
				{
					c8_incr();
				}
				c8_incr();
				break;
			case 0xA000:
				// ANNN set I to NNN
				cpu.i = opcode & 0x0FFF;
				c8_incr();
				break;
			case 0xB000:
				// BNNN jump to i + r0
				cpu.ip = (opcode & 0x0FFF) + cpu.v[0];
				break;
			case 0xC000:
				// CXNN set VX = rand() & NN
				cpu.v[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
				c8_incr();
				break;
			case 0xD000:
				// DXYN draw a sprite of 8-bit width, and arbitrary height at X, Y
				{
					int vx = cpu.v[(opcode & 0x0F00) >> 8];
					int vy = cpu.v[(opcode & 0x00F0) >> 4];
					int h = opcode & 0x000F;
					cpu.v[15] &= 0;
					
					for(int y = 0; y < h; y++)
					{
						int px = cpu.m[cpu.i + y];
						for(int x = 0; x < 8; x++)
						{
							if(px & (0x80 >> x))
							{
								if(cpu.px[x+vx+(y+vy)*C8_W])
									cpu.v[15] = 1;
								cpu.px[x+vx+(y+vy)*C8_W] ^= 1;
							}
						}
					}
				}
				c8_incr();
				break;
			case 0xE000:
				switch(opcode & 0x000F)
				{
					case 0x000E:
					// EX9E - skip instruction if key in VX is pressed
						{
							uint8_t* keys = SDL_GetKeyState(NULL);
							if(keys[c8_keymap[cpu.v[((opcode & 0x0F00) >> 8)] & 0x0F]])
							{
								c8_incr();
							}
						}
						c8_incr();
						break;
					case 0x0001:
						// EXA1 - skip instruction if key in VX is not pressed
						{
							uint8_t* keys = SDL_GetKeyState(NULL);
							if(!keys[c8_keymap[cpu.v[((opcode & 0x0F00) >> 8)] & 0x0F]])
							{
								c8_incr();
							}
						}
						c8_incr();
						break;
					default:
						c8_err("invalid instruction in binary: 0x%x\n", cpu.ip - C8_LOAD_POINT);
						break;
				}
				break;
			case 0xF000:
				switch(opcode & 0x00FF)
				{
					case 0x0007:
					// FX07 VX = DT
						cpu.v[(opcode & 0x0F00) >> 8] = cpu.dt;
						c8_incr();
						break;
					case 0x000A:
					// FX0A - A keypress is awaited, then store in VX
						{
							uint8_t* keys = SDL_GetKeyState(NULL);
							for(int i = 0; i < 0x10; i++)
								if(keys[c8_keymap[i]])
								{
									cpu.v[(opcode & 0x0F00) >> 8] = i;
									c8_incr();
								}
						}
						break;
					case 0x0015:
					// FX15 Sets DT = VX
						cpu.dt = cpu.v[(opcode & 0x0F00) >> 8];
						c8_incr();
						break;
					case 0x0018:
						cpu.st = cpu.v[(opcode & 0x0F00) >> 8];
						c8_incr();
						break;
					case 0x001E:
						// FX1E - i = i + vx
						cpu.i += cpu.v[(opcode & 0x0F00) >> 8];
						c8_incr();
						break;
					case 0x0029:
						// FX29 set I to point to character
						cpu.i = cpu.v[(opcode & 0x0F00) >> 8] * 5;
						c8_incr();
						break;
					case 0x0033:
						// FX33 stores BCD at I, I+1, I+2
						{
							uint8_t r = cpu.v[(opcode & 0x0F00) >> 8];
							cpu.m[cpu.i] = r / 100;
							cpu.m[cpu.i+1] = (r / 10) % 10;
							cpu.m[cpu.i+2] = r % 10;
						}
						c8_incr();
						break;
					case 0x0055:
						// store values from I -> I + x into r0 to rX
						{
							uint8_t reg_max = (opcode & 0x0F00) >> 8;
							for(int i = 0; i <= reg_max; i++)
							{
								cpu.m[cpu.i+i] = cpu.v[i];
							}
						}
						c8_incr();
						break;
					case 0x0065:
						// get values from I -> I + x into r0 to rX
						{
							uint8_t reg_max = (opcode & 0x0F00) >> 8;
							for(int i = 0; i <= reg_max; i++)
							{
								cpu.v[i] = cpu.m[cpu.i+i];
							}
						}
						c8_incr();
						break;
					default:
						c8_err("invalid instruction in binary: 0x%x\n", cpu.ip - C8_LOAD_POINT);
						break;
				}
				break;
			default:
				c8_err("invalid instruction in binary: 0x%x\n", cpu.ip - C8_LOAD_POINT);
				break;
		}
		c8_timers();
	}
	
}
// run chip8
void c8_run(char* q) {
	c8_zero();
	
	FILE* f = fopen(q, "rb");
	if(!f)
		c8_err("unable to open game file! %s\n", q);
	
	fread(&cpu.m[C8_LOAD_POINT], 1, C8_MEM - C8_LOAD_POINT, f);
	fclose(f);
	
	for(int i = 0; i < 80; ++i)
		cpu.m[i] = c8_font[i];
	
	cpu.ip = C8_LOAD_POINT;
	
	SDL_Event ev;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("c8 - a chip8 emulator\n", "c8");
	
	for(;;) {
		if(SDL_PollEvent(&ev))
			continue;
		
		if(ev.type == SDL_QUIT)
			c8_exit(0);
		
		c8_emulate_instruction();
		c8_draw();
		c8_refresh(&ev);
	}
}

int main(int argc, char** argv) {
	if(argc < 2)
	{
		printf("chip8 emulator. pass -h for help\n");
	}
	char* romfile = NULL;
	if(argc >= 2) {
		for(int i = 1; i < argc; i++) {
			if(!strcmp(argv[i], "-h")) {
				printf("---- chip8 emulator ----\n");
				printf("this emulator is licensed under the freeBSD license\n");
				printf("project page: https://github.com/cyclicintegral/c8\n");
				printf("options: -rom [chip8 ROM file]\n");
				printf("-------------------------\n");
			}
			else if(!strcmp(argv[i], "-rom"))
			{
				if(i < argc-1)
				{
					romfile = argv[i+1];
					break;
				}
				else 
				{
					printf("error: no rom file\n");
					return 0;
				}
			}
		}
	}
	if(romfile == NULL)
	{
		printf("error: no rom file. pass -h for help\n");
	}
	else {
		c8_run(romfile);
	}
	return 0;
}

