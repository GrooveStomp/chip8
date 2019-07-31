/******************************************************************************
  File: input.h
  Created: 2019-07-21
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file input.h
//!
//! This package provides a small input abstraction layer to separate
//! input processing from system emulation.
//! Input operates in the same thread as graphics, at the same frequency (30hz)
//! and calls back into the system emulation thread.

#ifndef INPUT_VERSION
#define INPUT_VERSION "0.1.0"

#include "SDL.h"

struct input;
struct system;

//! \brief Creates and initializes a new input object
//! \return The initialized input object
struct input *
InputInit();

//! \brief De-initializes and frees memory for the given input object
//! \param[in,out] input The initialized input object to be cleaned and reclaimed
void
InputDeinit(struct input *input);

//! \brief Input processing
//!
//! The main input processing routine. This interacts with SDL, contains the
//! key mapping between SDL keys and the CHIP-8 hex keyboard, and calls
//! synchronization routines on the system package to synchronize data
//! across threads.
//!
//! \param[in,out] input Input state to be updated
//! \param[in,out] system CHIP-8 System state to be updated
//! \param[in] event SDL event to process
void
InputCheck(struct input *input, struct system *system, SDL_Event *event);

#endif // INPUT_VERSION
