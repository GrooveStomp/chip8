#include "SDL.h"

#include "system.h"

#define NUM_KEYS 16

struct input {
        SDL_Keycode keycodeIndices[NUM_KEYS];
};

struct input *InputInit() {
        struct input *i = (struct input *)malloc(sizeof(struct input));
        memset(i, 0, sizeof(struct input));

        i->keycodeIndices[1] = SDLK_q;
        i->keycodeIndices[2] = SDLK_w;
        i->keycodeIndices[3] = SDLK_e;
        i->keycodeIndices[0xC] = SDLK_r;
        i->keycodeIndices[4] = SDLK_a;
        i->keycodeIndices[5] = SDLK_s;
        i->keycodeIndices[6] = SDLK_d;
        i->keycodeIndices[0xD] = SDLK_f;
        i->keycodeIndices[7] = SDLK_u;
        i->keycodeIndices[8] = SDLK_i;
        i->keycodeIndices[9] = SDLK_o;
        i->keycodeIndices[0xE] = SDLK_p;
        i->keycodeIndices[0xA] = SDLK_j;
        i->keycodeIndices[0] = SDLK_k;
        i->keycodeIndices[0xB] = SDLK_l;
        i->keycodeIndices[0xF] = SDLK_SEMICOLON;
}

void HandleKeyPress(struct input *input, struct system *s, SDL_Keycode k) {
        for (int i = 0; i < NUM_KEYS; i++) {
                if (k == input->keycodeIndices[i]) {
                        s->key[i] = 0xFF; // Pressed.
                        if (s->waitForKey != -1) {
                                s->v[s->waitForKey] = i;
                                s->waitForKey = -1;
                        }
                        break;
                }
        }
}

// Returns 0 if event normally processed. Non-zero indicates termination of program.
int InputCheck(struct input *i, struct system *s) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_QUIT:
                                return !0;
                                break;
                        case SDL_KEYUP:
                                break;
                        case SDL_KEYDOWN:
                                if (event.key.keysym.sym == SDLK_ESCAPE) {
                                        return !0;
                                }
                                HandleKeyPress(i, s, event.key.keysym.sym);
                                break;
                }
        }
}
