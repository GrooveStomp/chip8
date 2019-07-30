/******************************************************************************
  File: graphics.c
  Created: 2019-06-25
  Updated: 2019-07-30
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <string.h> // memset
#include <stdio.h> // fprintf

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#include "system.h"

const unsigned int DISPLAY_WIDTH_WITH_DEBUGGER = 1445;
const unsigned int DISPLAY_HEIGHT_WITH_DEBUGGER = 720;
const unsigned int CHIP8_DISPLAY_WIDTH = 64;
const unsigned int CHIP8_DISPLAY_HEIGHT = 32;
const unsigned int DISPLAY_SCALE = 16;

struct graphics {
        GLubyte *textureData;
        SDL_Window *sdlWindow;
        SDL_GLContext *glContext;
        unsigned int displayWidth;
        unsigned int displayHeight;
        int debug;
        int glWindowWidth;
        int glWindowHeight;
        GLuint glTextureName;
};

struct graphics *GraphicsInit(int debug) {
        struct graphics *g = (struct graphics *)malloc(sizeof(struct graphics));
        memset(g, 0, sizeof(struct graphics));

        g->debug = debug;
        g->textureData = (GLubyte *)malloc(CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3);

        if (debug) {
                g->displayWidth = DISPLAY_WIDTH_WITH_DEBUGGER;
                g->displayHeight = DISPLAY_HEIGHT_WITH_DEBUGGER;
        } else {
                g->displayWidth = CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE;
                g->displayHeight = CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE;
        }

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

        g->sdlWindow = SDL_CreateWindow(
                "AaronO's CHIP-8 Emulator",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                g->displayWidth, g->displayHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
                );

        if (g->sdlWindow == NULL) {
                fprintf(stderr, "Couldn't open window: %s\n", SDL_GetError());
                return NULL;
        }

        g->glContext = SDL_GL_CreateContext(g->sdlWindow);
        if (NULL == g->glContext) {
                fprintf(stderr, "%s\n", SDL_GetError());
                return NULL;
        }

        SDL_GetWindowSize(g->sdlWindow, &g->glWindowWidth, &g->glWindowHeight);
        glViewport(0, 0, g->glWindowWidth, g->glWindowHeight);

        if (glewInit() != GLEW_OK) {
                fprintf(stderr, "Failed to setup GLEW\n");
                SDL_DestroyWindow(g->sdlWindow);
                SDL_Quit();
                return NULL;
        }

        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &g->glTextureName);
        glBindTexture(GL_TEXTURE_2D, g->glTextureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)g->textureData);

        return g;
}

void GraphicsDeinit(struct graphics *g) {
        SDL_GL_DeleteContext(g->glContext);
        SDL_DestroyWindow(g->sdlWindow);
        SDL_Quit();

        free(g->textureData);
        free(g);
}

void Raster(struct graphics *g, struct system *s) {
        if (0 != SystemGfxLock(s)) {
                fprintf(stderr, "Failed to lock system gfx rwlock");
                return;
        }

        memset(g->textureData, 0xFF, CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3);

        for (int y = CHIP8_DISPLAY_HEIGHT-1, cy = 0; cy < CHIP8_DISPLAY_HEIGHT; cy++, y--) {
                for (int x = 0, cx = 0; cx < CHIP8_DISPLAY_WIDTH; cx++, x+=3) {
                        unsigned int pos = cy * (CHIP8_DISPLAY_WIDTH * 3) + x;

                        if (s->gfx[y * CHIP8_DISPLAY_WIDTH + cx]) {
                                // Black (Foreground)
                                g->textureData[pos + 0] = 0;
                                g->textureData[pos + 1] = 0;
                                g->textureData[pos + 2] = 0;
                        }
                }
        }
        SystemGfxUnlock(s);
}

void GraphicsPresent(struct graphics *g, struct system *s, void (*ui_render_fn)()) {
        SDL_GetWindowSize(g->sdlWindow, &g->glWindowWidth, &g->glWindowHeight);
        glViewport(0, 0, g->glWindowWidth, g->glWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.10f, 0.18f, 0.24f, 1.0f);
        ui_render_fn();

        glOrtho(-1, 1, -1, 1, -1, 1);
        glColor3f(1, 1, 1);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, g->glTextureName);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        Raster(g, s);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)g->textureData);

        float top, bottom, left, right;
        if (g->debug) {
                top = 0.25;
                bottom = -0.75;
                left = 0;
                right = 1;
        } else {
                top = 1;
                bottom = -1;
                left = -1;
                right = 1;
        }
        glBegin(GL_TRIANGLES); {
                glTexCoord2f(0, 0);
                glVertex3f(left, bottom, 0.5);
                glTexCoord2f(0, 1);
                glVertex3f(left, top, 0.5);
                glTexCoord2f(1, 0);
                glVertex3f(right, bottom, 0.5);

                glTexCoord2f(0, 1);
                glVertex3f(left, top, 0.5);
                glTexCoord2f(1, 1);
                glVertex3f(right, top, 0.5);
                glTexCoord2f(1, 0);
                glVertex3f(right, bottom, 0.5);
        } glEnd();

        SDL_GL_SwapWindow(g->sdlWindow);
}

SDL_Window *GraphicsSDLWindow(struct graphics *g) {
        return g->sdlWindow;
}
