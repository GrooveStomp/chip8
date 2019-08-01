/******************************************************************************
  File: ui.h
  Created: 2019-06-27
  Updated: 2019-08-01
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
//! \file ui.h
//!
//! The UI system uses nuklear internally for an immediate-mode UI.
//! The UI isn't a general purpose UI, rather it's a graphical debugger embedded
//! within the CHIP-8 emulator.
//!
//! \image html debugger-ui.png
//!
//! \see Nuklear: https://github.com/vurtun/nuklear
//! \see Immediate mode GUI: https://caseymuratori.com/blog_0001

#ifndef UI_VERSION
//! include guard
#define UI_VERSION "0.1.0"

#include "SDL2/SDL.h"

struct system;
struct opcode;
struct ui;

//! \brief Creates and initializes a new ui object instance
//! \param[in] shouldBeEnabled whether the UI will be rendered or not
//! \param[in] widgetWidth standard width of UI widgets
//! \param[in] widgetHeight standard height of UI widgets
//! \param[in] window SDL Window initialized by the graphics subsystem
//! \return The initialized ui object
struct ui *
UIInit(int shouldBeEnabled, unsigned int widgetWidth, unsigned int widgetHeight, SDL_Window *window);

//! \brief De-initializes and frees memory for the given ui object
//! \param[in,out] ui The initialized ui object to be cleaned and reclaimed
void
UIDeinit(struct ui *ui);

//! \brief Updates state of the UI to allow input processing
//!
//! This needs to be called every frame before UIHandleEvent() and should
//! be concluded with a call to UIInputEnd()
//!
//! \param[in,out] ui UI state to be updated
void
UIInputBegin(struct ui *ui);

//! \brief Updates state of the UI to stop input processing
//!
//! This needs to be called every frame after UIHandleEvent() and should be
//! preceded with a call to UIInputBegin()
//!
//! \param[in,out] ui UI state to be updated
void
UIInputEnd(struct ui *ui);

//! \brief Processes SDL input events
//!
//! This needs to be called every frame between UIInputBegin() and UIInputEnd()
//!
//! \param[in,out] ui UI state to be updated
//! \param[in] event The SDL_Event that occurred and should be processed
void
UIHandleEvent(struct ui *ui, SDL_Event *event);

//! \brief Sets up all UI widgets before rendering
//!
//! This needs to be called every frame and should be succeeded with a call to
//! UIRender().
//!
//! NOTE: UIRender() is invoked in gfxinputthread.c via a wrapper function:
//! UIRenderFn().  This is done to simplify the graphics API.
//!
//! \param[in,out] ui UI state to be updated
//! \param[in] system system state to be read and presented in the ui
//! \param[in] opcode opcode state to be read and presented in the ui
void
UIWidgets(struct ui *ui, struct system *system, struct opcode *opcode);

//! \brief Renders the ui
//!
//! Uses the SDL_Window used in UIInit() to render the configured UI widgets.
//! This must be called every frame aftr UIWidgets()
//!
//! \param[in,out] ui UI state to be used for rendering
void
UIRender(struct ui *ui);

#endif // UI_VERSION
