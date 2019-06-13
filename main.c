/*
  Chip-8 Emulator (First Emulation Project)
*/

#include <stdio.h>
#include <malloc.h>

#include "system.h"
#include "opcode.c"

void ArgParse(int argc, char **argv, int debug) {
        if (debug) {
                printf("argc: %d, argv: ", argc);
                for (int i=0; i< argc; i++) {
                        printf("%s ", argv[i]);
                }
                printf("\n");
        }

        if (argc != 2) {
                printf("chip-8 PROGRAM\n");
                exit(1);
        }
}

int main(int argc, char **argv) {
        // TODO: Initialize graphics subsystem.

        struct opcode *opcode = (struct opcode*)malloc(sizeof(struct opcode));
        OpcodeInit(opcode);

        struct system *system = (struct system*)malloc(sizeof(struct system));
        SystemInit(system);

        // TODO: Input system.
        //        struct input *input = (struct input*)malloc(sizeof(struct input));
        //        InputInit(input);

        ArgParse(argc, argv, 0);

        size_t fsize = 0;
        char *mem;
        {
                FILE *f = fopen(argv[1], "r");
                if (f == NULL) {
                        perror("Couldn't open file");
                        exit(1);
                }

                int c;
                while ((c = fgetc(f)) != EOF) {
                        fsize++;
                }

                if (ferror(f)) {
                        perror("Stopped reading file early");
                }

                clearerr(f);
                fseek(f, 0, SEEK_SET);

                int i = 0;
                mem = malloc(fsize);
                while ((c = fgetc(f)) != EOF) {
                        mem[i] = (char)c;
                        i++;
                }

                fclose(f);
        }
        if (!SystemLoadProgram(system, mem, fsize)) {
                printf("Couldn't load program into Chip-8\n");
        }

        // Emulation loop
        for(int i=0; i<256; i++) { // TODO: Run full program, not just 256 iterations.
                // TODO: Run on some frequency, not just as fast as possible.
                // Emulate one cycle:
                OpcodeFetch(opcode, system);
                OpcodeDecode(opcode);
                OpcodeExecute(opcode, system);
                // TODO: Update Timers

                // If the draw flag is set, update the screen
                // if(myChip8.drawFlag)
                //        drawGraphics();

                // Store key press state (Press and Release)
                // myChip8.setKeys();
        }

        return 0;
}
