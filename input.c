/******************************************************************************
  File: input.c
  Created: 2019-06-21
  Updated: 2019-07-25
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include "SDL2/SDL.h"
#include "system.h"

#define NUM_KEYS 16

struct input {
        SDL_Keycode keycodeIndices[NUM_KEYS];
};

typedef void *(*allocator)(size_t);
typedef void (*deallocator)(void *);

static allocator ALLOCATOR = malloc;
static deallocator DEALLOCATOR = free;

void InputMemControl(allocator Alloc, deallocator Dealloc) {
        ALLOCATOR = Alloc;
        DEALLOCATOR = Dealloc;
}

struct input *InputInit() {
        struct input *i = (struct input *)ALLOCATOR(sizeof(struct input));
        memset(i, 0, sizeof(struct input));

        // TODO: Support remapping of keys.
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

        return i;
}

void InputDeinit(struct input *i) {
        if (NULL == i)
                return;

        DEALLOCATOR(i);
}

void HandleKeyDown(struct input *input, struct system *s, SDL_Keycode k) {
        if (NULL == input)
                return;

        for (int i = 0; i < NUM_KEYS; i++) {
                if (k == input->keycodeIndices[i]) {
                        s->key[i] = 0xFF; // Pressed.
                        if (SystemWFKWaiting(s)) {
                                SystemWFKOccurred(s, i);
                        }
                        break;
                }
        }
}

void HandleKeyUp(struct input *input, struct system *s, SDL_Keycode k) {
        if (NULL == input)
                return;

        for (int i = 0; i < NUM_KEYS; i++) {
                if (k == input->keycodeIndices[i]) {
                        s->key[i] = 0x00; // Un-Pressed.
                        break;
                }
        }
}

void InputCheck(struct input *i, struct system *s, SDL_Event *event) {
        if (NULL == i)
                return;

        switch (event->type) {
                case SDL_QUIT:
                        SystemSignalQuit(s);
                        break;

                case SDL_KEYUP:
                        HandleKeyUp(i, s, event->key.keysym.sym);
                        break;

                case SDL_KEYDOWN:
                        if (event->key.keysym.sym == SDLK_ESCAPE) {
                                SystemSignalQuit(s);
                                break;
                        }
                        HandleKeyDown(i, s, event->key.keysym.sym);
                        break;
        }
}
