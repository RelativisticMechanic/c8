# c8

The CHIP-8 was a virtual machine designed in the 1970s by Joseph Weisbecker to make it easier for programs to be developed on the COSMAC VIP. Due to its simplicity, as the machine design was simple and the instructions are few, it has been used as a way to teach programmers how CPUs actually work at a very low, bytecode level.

## CPU Specification
The CHIP-8 has 4096 bytes of memory (both code & data). It has 16 8-bit data registers, numbered V0 to VF where VF operates as the FLAGS register. Finally, there is the I, or the memory indexing register that is used to address 4K of memory.

There is no stack, or rather, it is inaccesible. The stack is only used to conserve state of return addresses for the CALL-RET pair.

Finally, we've two timers, both operating at 60 Hz.

- Delay Timer: Can be set and read. Used for timing events in games.
- Sound Timer: Can be set. When it reaches zero, a beep is made.

## Input

The CHIP-8 input controller is a hexadecimal keyboard, with a 4x4 matrix arrangement of keys numbered 0 - F. Three opcodes are used to detect input. One skips an instruction if a specific key is pressed, while another does the same if a specific key is not pressed. The third waits for a key press, and then stores it in one of the data registers.

## Display

The Display resolution is 64×32 pixels, and its a black and white screen, no so RGB just 0s and 1s! Graphics are drawn to the screen solely by drawing sprites, which are 8 pixels wide and may be from 1 to 15 pixels in height. Sprite pixels are XOR'd with corresponding screen pixels. In other words, sprite pixels that are set flip the color of the corresponding screen pixel, while unset sprite pixels do nothing. The carry flag (VF) is set to 1 if any screen pixels are flipped from set to unset when a sprite is drawn and set to 0 otherwise. This is used for collision detection.

## Instruction Set

The complete instruction set for the CHIP-8 can be found on Wikipedia [here](https://en.wikipedia.org/wiki/CHIP-8#Opcode_table).
## Building and Running

Simply run

```
make
./c8 TETRIS.ROM
```

## Screenshots
![Screenshot](https://i.imgur.com/NcFekgI.png)

---

Have fun, experimenting!

Regards,

Siddharth Gautam.
