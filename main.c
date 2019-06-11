/*
  Chip-8 Emulator (First Emulation Project)
*/

#include <stdio.h>
#include <malloc.h>

#include "system.c"
#include "opcode.c"

// TODO: Not sure where to place this yet. (ie., in system.c?)
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

int main(int argc, char **argv) {
        // Set up render system and register input callbacks
        // setupGraphics();
        // setupInput();

        struct opcode *opcode = (struct opcode*)malloc(sizeof(struct opcode));
        OpcodeInit(opcode);

        struct system *system = (struct system*)malloc(sizeof(struct system));
        SystemInit(system);
        // Initialize the Chip8 system and load the game into the memory

        // myChip8.initialize();
        // myChip8.loadGame("pong");

        // Emulation loop
        // for(;;)
        {
                // Emulate one cycle
                // myChip8.emulateCycle();
                OpcodeFetch(opcode, system);
                OpcodeDecode(opcode);
                OpcodeExecute(opcode, system);

                // If the draw flag is set, update the screen
                // if(myChip8.drawFlag)
                //        drawGraphics();

                // Store key press state (Press and Release)
                // myChip8.setKeys();
        }

        return 0;
}
