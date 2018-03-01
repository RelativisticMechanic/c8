#ifndef CPU8_H
#define CPU8_H

#include <stdint.h>

#define SCREEN_H 320
#define SCREEN_W 640

#define C8_W 64
#define C8_H 32

#define C8_MEM 4096
#define C8_LOAD_POINT 0x200

typedef struct {
	uint8_t m[C8_MEM]; // memory
	uint8_t v[16]; // regs
	uint16_t ip; // instruction ptr
	uint16_t i; // index reg
	
	uint16_t stk[256]; // stack
	uint8_t sp; // stack ptr
	
	uint8_t dt, st; // delay and sound timers
	uint8_t inp[16]; // input
	uint8_t px[C8_W*C8_H]; // graphics output
} c8;

#endif
