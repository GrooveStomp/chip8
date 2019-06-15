/*
  Chip-8 Emulator (First Emulation Project)
*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "system.h"
#include "opcode.h"
// #include "input.h"
// #include "graphics.h"

// static const double MS_PER_FRAME = 0.03333333; // 30 FPS
static const double MS_PER_FRAME = 0.01666666; // 60 FPS
static int DEBUG_MODE = 0;
static char *PROGRAM = NULL;

void Usage() {
        printf("chip-8 [-d] PROGRAM\n");
        printf("\t-d: interactive debug mode\n");
}

void ArgParse(int argc, char **argv, int debug) {
        if (debug) {
                printf("argc: %d, argv: ", argc);
                for (int i=0; i< argc; i++) {
                        printf("%s ", argv[i]);
                }
                printf("\n");
        }

        if (argc < 2 || argc > 3) {
                Usage();
                exit(1);
        }

        if (argc == 3 && strcmp(argv[1], "-d") == 0) {
                argv[1] = argv[2];
                DEBUG_MODE = 1;
        } else if (argc == 3 && strcmp(argv[2], "-d") == 0) {
                DEBUG_MODE = 1;
        } else if (argc == 3) {
                Usage();
                exit(1);
        }
}

int main(int argc, char **argv) {
        // TODO: sruct graphics *graphics = GraphicsInput();
        struct opcode *opcode = OpcodeInit();
        struct system *system = SystemInit();
        // TODO: struct input *input = InputInit();

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

        for (;;) {
                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                OpcodeFetch(opcode, system);
                OpcodeDecode(opcode);

                if (DEBUG_MODE) {
                        OpcodePrint(opcode);
                        SystemDebug(system);
                        char in[256];
                        fgets(in, 256, stdin);
                }

                OpcodeExecute(opcode, system);

                SystemDecrementTimers(system);

                // TODO: GraphicsRender(graphics, system);
                // TODO: InputProcess(input, system);

                if (!DEBUG_MODE) {
                        struct timespec end;
                        clock_gettime(CLOCK_REALTIME, &end);

                        double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                        elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                        struct timespec sleep = { .tv_sec = 0, .tv_nsec = (MS_PER_FRAME - elapsed_time) * 1000 };
                        nanosleep(&sleep, NULL);
                }
        }

        return 0;
}
