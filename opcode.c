/*
  Chip-8 opcode
*/

#ifndef OPCODE_C
#define OPCODE_C

#include <stdlib.h> // rand
#include <limits.h> // UINT_MAX

#include "system.c"

struct opcode;

typedef void (*opcode_fn)(struct opcode *c, struct system *);

// All instructions are 2 bytes store most-significant byte first. (Big Endian)
struct opcode {
        unsigned short instruction;
        opcode_fn fn;
};

void __opcode_Debug(struct opcode *c) {
        printf("struct opcode {\n");
        printf("\tunsigned short instruction = 0x%04X;\n", c->instruction);
        printf("\topcode_fn fn = %p;\n", c->fn);
        printf("};\n");
}

void OpcodeInit(struct opcode *c) {
        c->instruction = 0;
        c->fn = NULL;
}

unsigned int __opcode_HighByte(struct opcode *c) {
        return (c->instruction >> 8) & 0xFF;
}

unsigned int __opcode_LowByte(struct opcode *c) {
        return c->instruction & 0xFF;
}

// pos: LSB first. 0 is LSB, 1 is next most significant byte, etc.
unsigned int __opcode_NibbleAt(struct opcode *c, unsigned int pos) {
        switch (pos) {
                case 0:
                        return (c->instruction & 0x000F);
                        break;
                case 1:
                        return (c->instruction & 0x00F0) >> 4;
                        break;
                case 2:
                        return (c->instruction & 0x0F00) >> 8;
                        break;
                case 3:
                        return (c->instruction & 0xF000) >> 16;
                        break;
        }

        return UINT_MAX;
}

// See full fn listing: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

void __opcode_0NNN(struct opcode *c, struct system *s) {
        // type: Call
        // C Pseudo:
        // Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
}

void __opcode_00E0(struct opcode *c, struct system *s) {
        // type: Display
        // C Pseudo: disp_clear()
        // Clears the screen.
}

void __opcode_00EE(struct opcode *c, struct system *s) {
        // type: Flow
        // C Pseudo: return;
        // Returns from a subroutine.
}

// Flow control: goto NNNN;
void __opcode_1NNN(struct opcode *c, struct system *s) {
        unsigned int low_byte = __opcode_LowByte(c);
        unsigned int nibble = __opcode_NibbleAt(c, 2);
        unsigned int address = ((nibble << 3) | address);
        s->pc = address;
}

// Flow control: Call subroutine at NNN;
void __opcode_2NNN(struct opcode *c, struct system *s) {
        unsigned int low_byte = __opcode_LowByte(c);
        unsigned int nibble = __opcode_NibbleAt(c, 2);
        unsigned int address = ((nibble << 3) | address);
        // TODO
}

// Condition: Skip next instruction if VX equals NN.
void __opcode_3XNN(struct opcode *c, struct system *s) {
        unsigned int nn = __opcode_LowByte(c);
        unsigned int x = __opcode_NibbleAt(c, 2);

        if (s->v[x] == nn) {
                SystemIncrementPC(s);
        }
}

// Condition: Skip next instruction if VX doesn't equal NN.
void __opcode_4XNN(struct opcode *c, struct system *s) {
        unsigned int nn = __opcode_LowByte(c);
        unsigned int x = __opcode_NibbleAt(c, 2);

        if (s->v[x] != nn) {
                SystemIncrementPC(s);
        }
}

// Condition: Skip next instruction if VX equals VY.
void __opcode_5XY0(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        if (s->v[x] == s->v[y]) {
                SystemIncrementPC(s);
        }
}

// Constant expression: Sets VX to NN.
void __opcode_6XNN(struct opcode *c, struct system *s) {
        unsigned int nn = __opcode_LowByte(c);
        unsigned int x = __opcode_NibbleAt(c, 2);

        s->v[x] = nn;
}

// Constant expression: Adds NN to VX (carry flag is not changed).
void __opcode_7XNN(struct opcode *c, struct system *s) {
        unsigned int nn = __opcode_LowByte(c);
        unsigned int x = __opcode_NibbleAt(c, 2);

        s->v[x] += nn;
}

// Assignment: Sets VX to the value of VY.
void __opcode_8XY0(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        s->v[x] = s->v[y];
}

// Bitwise Operation: Sets VX to: VX | VY.
void __opcode_8XY1(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        s->v[x] = s->v[x] | s->v[y];
}

// Bitwise Operation: Sets VX to: VX & VY.
void __opcode_8XY2(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        s->v[x] = s->v[x] & s->v[y];
}

// Bitwise Operation: Sets VX to: VX ^ VY.
void __opcode_8XY3(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        s->v[x] = s->v[x] ^ s->v[y];
}

// Math: Adds VY to VX. VF is set to 1 when there's a carry and 0 otherwise.
void __opcode_8XY4(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        int val = s->v[x] + s->v[y];
        s->v[x] = (unsigned char)val;

        if ((int)(s->v[x]) != val) {
                s->v[15] = 1;
        }
}

// Math: VY is subtracted from VX. VF is set to 0 when there's a borrow and 1 otherwise.
void __opcode_8XY5(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        int val = s->v[x] - s->v[y];
        s->v[x] = (unsigned char)val;

        s->v[15] = 1;
        if ((int)(s->v[x]) != val) {
                s->v[15] = 0;
        }
}

// Bitwise Operation: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
void __opcode_8XY6(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        unsigned char lsb = s->v[x] | 0x01;
        s->v[15] = lsb;
        s->v[x] >> 1;
}

// Math: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
void __opcode_8XY7(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        int val = s->v[y] - s->v[x];
        s->v[x] = (unsigned char)val;

        s->v[15] = 1;
        if ((int)(s->v[x]) != val) {
                s->v[15] = 0;
        }
}

// Bitwise Operation: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
void __opcode_8XYE(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        unsigned char msb = s->v[x] | 0x80;
        s->v[15] = msb;
        s->v[x] << 1;
}

// Condition: Skips the next instruction if VX doesn't equal VY.
void __opcode_9XY0(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned int y = __opcode_NibbleAt(c, 1);

        if (s->v[x] != s->v[y]) {
                SystemIncrementPC(s);
        }
}

// Memory: Sets I to the address NNN.
void __opcode_ANNN(struct opcode *c, struct system *s) {
        unsigned int low_byte = __opcode_LowByte(c);
        unsigned int nibble = __opcode_NibbleAt(c, 2);
        unsigned int address = ((nibble << 3) | low_byte);

        s->i = address;
}

// Flow control: Jumps to the address NNN plus V0.
void __opcode_BNNN(struct opcode *c, struct system *s) {
        unsigned int low_byte = __opcode_LowByte(c);
        unsigned int nibble = __opcode_NibbleAt(c, 2);
        unsigned int address = ((nibble << 3) | low_byte);

        s->i = (s->v[0] + address);
}

// Random: Sets VX to the result of a bitwise AND on a random number (0-255) and NN.
void __opcode_CXNN(struct opcode *c, struct system *s) {
        unsigned int nn = __opcode_LowByte(c);
        unsigned int x = __opcode_NibbleAt(c, 2);

        int r = (unsigned int)(rand() % 255);
        s->v[x] = nn & r;
}

// Display: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
// and a height of N pixels. Each row of 8 pixels is read as bit-coded starting
// from memory location I; I value doesn’t change after the execution of this
// instruction. As described above, VF is set to 1 if any screen pixels are
// flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t
// happen.
// I'm assuming (VX, VY) is the lower-left corner of the sprint, not the center.
void __opcode_DXYN(struct opcode *c, struct system *s) {
        unsigned int x_pos = __opcode_NibbleAt(c, 2);
        unsigned int y_pos = __opcode_NibbleAt(c, 1);
        unsigned int height = __opcode_NibbleAt(c, 0);

        s->v[15] = 0;

        for (int y = 0; y < height; y++) {
                // I contains a 1-byte bitmap representing a line of the sprite.
                // [XXXX XXXX]
                unsigned char pixel = s->memory[s->i + y];

                for (int x = 0; x < 8; x++) {
                        if ((pixel & (0x80 >> x)) == 0) { // This line taken from www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
                                continue;
                        }

                        int y_off = (y_pos + y) * 64;
                        int x_off = (x_pos + x);
                        int pos = y_off + x_off;

                        if (s->gfx[pos] == 1) {
                                s->v[15] = 1;
                        }

                        s->gfx[pos] ^= 1;
                }
        }
}

// Key operation: Skips the next instruction if the key stored in VX is pressed.
void __opcode_EX9E(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned char key = s->v[x];

        if (s->key[key]) {
                SystemIncrementPC(s);
        }
}

// Key operation: Skips the next instruction if the key stored in VX isn't pressed.
void __opcode_EXA1(struct opcode *c, struct system *s) {
        unsigned int x = __opcode_NibbleAt(c, 2);
        unsigned char key = s->v[x];

        if (!(s->key[key])) {
                SystemIncrementPC(s);
        }
}

// Stores two-byte opcode from memory pointed to by pc into opcode c.
void OpcodeFetch(struct opcode *c, struct system *s) {
        // Fetch the first byte from memory.
        // Left shift 8 bits, padding on the right with zeroes.
        // Binary OR with the next byte from memory.
        // eg.:
        //     A200
        //  OR 00A2
        //  =======
        //     A2A2
        c->instruction = s->memory[s->pc] << 8 | s->memory[s->pc + 1];
}

// TODO: Use a proper hashing routine and do fast lookup?
//       Although, with just 35 values we're probably okay?
void OpcodeDecode(struct opcode *c) {
        int msb = __opcode_NibbleAt(c, 3);

        switch (msb) {
                case 0: {
                        // TODO
                } break;

                case 1: {
                        c->fn = __opcode_1NNN;
                } break;

                case 2: {
                        c->fn = __opcode_2NNN;
                } break;

                case 3: {
                        c->fn = __opcode_3XNN;
                } break;

                case 4: {
                        c->fn = __opcode_4XNN;
                } break;

                case 5: {
                        c->fn = __opcode_5XY0;
                } break;

                case 6: {
                        c->fn = __opcode_6XNN;
                } break;

                case 7: {
                        c->fn = __opcode_7XNN;
                } break;

                case 8: {
                        unsigned int low_bit = __opcode_NibbleAt(c, 0);
                        switch (low_bit) {
                                case 0: {
                                        c->fn = __opcode_8XY0;
                                } break;

                                case 1: {
                                        c->fn = __opcode_8XY1;
                                } break;

                                case 2: {
                                        c->fn = __opcode_8XY2;
                                } break;

                                case 3: {
                                        c->fn = __opcode_8XY3;
                                } break;

                                case 4: {
                                        c->fn = __opcode_8XY4;
                                } break;

                                case 5: {
                                        c->fn = __opcode_8XY5;
                                } break;

                                case 6: {
                                        c->fn = __opcode_8XY6;
                                } break;

                                case 7: {
                                        c->fn = __opcode_8XY7;
                                } break;

                                case 0xE: {
                                        c->fn = __opcode_8XYE;
                                } break;
                        }
                } break;

                case 9: {
                        c->fn = __opcode_9XY0;
                } break;

                case 0xA: {
                        c->fn = __opcode_ANNN;
                } break;

                case 0xB: {
                        c->fn = __opcode_BNNN;
                } break;

                case 0xC: {
                        c->fn = __opcode_CXNN;
                } break;

                case 0xD: {
                        c->fn = __opcode_DXYN;
                } break;

                case 0xE: {
                        unsigned int low_byte = __opcode_LowByte(c);
                        switch (low_byte) {
                                case 0X9E: {
                                        c->fn = __opcode_EX9E;
                                } break;
                                case 0xA1: {
                                        c->fn = __opcode_EXA1;
                                } break;
                        }
                } break;

                case 0xF: {
                        // TODO
                } break;
        }

        printf("Unknown opcode: %#X\n", c->instruction);
}

void OpcodeExecute(struct opcode *c, struct system *s) {
        // TODO: What happens if c->fn is NULL? Consume as normal or abort?
        //       Probably abort!
        if (c->fn == NULL) {
                __opcode_Debug(c);
                return;
        }

        c->fn(c, s);

        SystemIncrementPC(s);
}

#endif // OPCODE_C
