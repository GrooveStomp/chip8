/******************************************************************************
  File: input.c
  Created: 2019-06-21
  Updated: 2019-08-03
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include "SDL2/SDL.h"
#include "system.h"
//! \file input.c

//! The number of keys in the CHIP-8
#define NUM_KEYS 16

//! \brief Keypress state. Unexported.
struct input {
        SDL_Keycode keycodeIndices[NUM_KEYS];
};

struct input *InputInit() {
        struct input *i = (struct input *)malloc(sizeof(struct input));
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

        free(i);
}

//! \brief Key "down" event handler
//! \param[in,out] input Input state to be updated
//! \param[in,out] system CHIP-8 system state to be updated
//! \param[in] keycode Keycode mapping which key has been pressed
void HandleKeyDown(struct input *input, struct system *system, SDL_Keycode keycode) {
        if (NULL == input)
                return;

        for (int i = 0; i < NUM_KEYS; i++) {
                if (keycode == input->keycodeIndices[i]) {
                        SystemKeySetPressed(system, i, 1);
                        if (SystemWFKWaiting(system)) {
                                SystemWFKOccurred(system, i);
                        }
                        break;
                }
        }
}

//! \brief Key "up" event handler
//! \param[in,out] input Input state to be updated
//! \param[in,out] system CHIP-8 system state to be updated
//! \param[in] keycode Keycode maping which key has been released
void HandleKeyUp(struct input *input, struct system *system, SDL_Keycode keycode) {
        if (NULL == input)
                return;

        for (int i = 0; i < NUM_KEYS; i++) {
                if (keycode == input->keycodeIndices[i]) {
                        SystemKeySetPressed(system, i, 0);
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
