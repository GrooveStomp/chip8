/******************************************************************************
  File: graphics.h
  Created: 2019-07-16
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file graphics.h

#ifndef GRAPHICS_VERSION
#define GRAPHICS_VERSION "0.1.0"

#include "SDL2/SDL.h"

struct system;
struct ui;

//! \brief Creates and initializes a new graphics object isntance
//! \param[in] debug Whether to enabled the debugging UI
//! \return The initialized graphics object
struct graphics *
GraphicsInit(int debug);

//! \brief De-initializes and frees memory for the given graphics object
//! \param[in,out] graphics The initialized opcode object to be cleaned and reclaimed
void
GraphicsDeinit(struct graphics *graphics);

//! \brief Gets the SDL_Window used by the graphics object
//! \param[in] graphics Graphics state to be read
//! \return Pointer to the SDL_Window used by graphics
SDL_Window *
GraphicsSDLWindow(struct graphics *graphics);

//! \brief Render the CHIP-8's video memory to the screen
//! \param graphics Graphics state to be used for rendering
//! \param system CHIP-8 system state to be read
//! \param ui_render_fn Function pointer used to render the UI
void
GraphicsPresent(struct graphics *graphics, struct system *system, void (*ui_render_fn)());

#endif // GRAPHICS_VERSION
