/*
  Chip-8 Emulator (First Emulation Project)
*/

#include <stdio.h>
#include <malloc.h>

#include "system.c"
#include "opcode.c"


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

        printf("Hello!\n");
        return 0;
}
