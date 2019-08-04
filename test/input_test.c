/******************************************************************************
  File: input_test.c
  Created: 2019-08-04
  Updated: 2019-08-04
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>

#include "gstest.h"

#include "../input.h"
#include "../input.c"
#include "../system.h"

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

char *TestInputInit() {
        struct input *input = InputInit();

        GSTestAssert(input->keycodeIndices[1] == SDLK_q, "Expected hex key 0x%02x to map to %d, but got %d", 1, SDLK_q, SDLK_q);
        GSTestAssert(input->keycodeIndices[2] == SDLK_w, "Expected hex key 0x%02x to map to %d, but got %d", 2, SDLK_w, SDLK_w);
        GSTestAssert(input->keycodeIndices[3] == SDLK_e, "Expected hex key 0x%02x to map to %d, but got %d", 3, SDLK_e, SDLK_e);
        GSTestAssert(input->keycodeIndices[0xC] == SDLK_r, "Expected hex key 0x%02x to map to %d, but got %d", 0xC, SDLK_r, SDLK_r);
        GSTestAssert(input->keycodeIndices[4] == SDLK_a, "Expected hex key 0x%02x to map to %d, but got %d", 4, SDLK_a, SDLK_a);
        GSTestAssert(input->keycodeIndices[5] == SDLK_s, "Expected hex key 0x%02x to map to %d, but got %d", 5, SDLK_s, SDLK_s);
        GSTestAssert(input->keycodeIndices[6] == SDLK_d, "Expected hex key 0x%02x to map to %d, but got %d", 6, SDLK_d, SDLK_d);
        GSTestAssert(input->keycodeIndices[0xD] == SDLK_f, "Expected hex key 0x%02x to map to %d, but got %d", 0xD, SDLK_f, SDLK_f);
        GSTestAssert(input->keycodeIndices[7] == SDLK_u, "Expected hex key 0x%02x to map to %d, but got %d", 7, SDLK_u, SDLK_u);
        GSTestAssert(input->keycodeIndices[8] == SDLK_i, "Expected hex key 0x%02x to map to %d, but got %d", 8, SDLK_i, SDLK_i);
        GSTestAssert(input->keycodeIndices[9] == SDLK_o, "Expected hex key 0x%02x to map to %d, but got %d", 9, SDLK_o, SDLK_o);
        GSTestAssert(input->keycodeIndices[0xE] == SDLK_p, "Expected hex key 0x%02x to map to %d, but got %d", 0xE, SDLK_p, SDLK_p);
        GSTestAssert(input->keycodeIndices[0xA] == SDLK_j, "Expected hex key 0x%02x to map to %d, but got %d", 0xA, SDLK_j, SDLK_j);
        GSTestAssert(input->keycodeIndices[0] == SDLK_k, "Expected hex key 0x%02x to map to %d, but got %d", 0, SDLK_k, SDLK_k);
        GSTestAssert(input->keycodeIndices[0xB] == SDLK_l, "Expected hex key 0x%02x to map to %d, but got %d", 0xB, SDLK_l, SDLK_l);
        GSTestAssert(input->keycodeIndices[0xF] == SDLK_SEMICOLON, "Expected hex key 0x%02x to map to %d, but got %d", 0xF, SDLK_SEMICOLON, SDLK_SEMICOLON);

        InputDeinit(input);
        return NULL;
}

char *TestInputDeinit() {
        // TODO: Any way to realistically test this?
        return NULL;
}

char *TestInputCheck_Quit(struct input *input, struct system *system) {
        SDL_Event event = { .type = SDL_QUIT };
        InputCheck(input, system, &event);
        GSTestAssert(SystemShouldQuit(system), "got %d, wanted %d", SystemShouldQuit(system), !0);

        return NULL;
}

char *TestInputCheck_Escape(struct input *input, struct system *system) {
        SDL_Event event = { .type = SDL_KEYDOWN };
        event.key.keysym.sym = SDLK_ESCAPE;
        InputCheck(input, system, &event);
        GSTestAssert(SystemShouldQuit(system), "got %d, wanted %d", SystemShouldQuit(system), !0);

        return NULL;
}

char *TestInputCheck_R(struct input *input, struct system *system) {
        SDL_Event event = { .type = SDL_KEYDOWN };

        event = (SDL_Event){ .type = SDL_KEYDOWN };
        event.key.keysym.sym = SDLK_r;
        InputCheck(input, system, &event);
        GSTestAssert(SystemKeyIsPressed(system, 0xC), "got %d, wanted %d", SystemKeyIsPressed(system, 0xC), !0);

        event = (SDL_Event){ .type = SDL_KEYUP };
        event.key.keysym.sym = SDLK_r;
        InputCheck(input, system, &event);
        GSTestAssert(SystemKeyIsPressed(system, 0xC) == 0, "got %d, wanted %d", SystemKeyIsPressed(system, 0xC), 0);

        return NULL;
}

char *TestInputCheck() {
        char *(*subtest[])(struct input *, struct system *) = {
                TestInputCheck_Quit,
                TestInputCheck_Escape,
                TestInputCheck_R
        };

        for (int i = 0; i < ARRAY_LENGTH(subtest); i++) {
                struct system *system = SystemInit(0);
                struct input *input = InputInit();
                char *result = subtest[i](input, system);
                if (result != NULL) {
                        return result;
                }
                InputDeinit(input);
                SystemDeinit(system);
        }

        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestInputInit);
        GSTestRun(TestInputDeinit);
        GSTestRun(TestInputCheck);
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("input_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
