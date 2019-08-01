/******************************************************************************
  File: sound.h
  Created: 2019-07-07
  Updated: 2019-08-01
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef SOUND_VERSION
#define SOUND_VERSION "0.1.0"

//! \file sound.h
//!
//! A very small interface to sound playback.
//!
//! Extremely limited in scope; currently only supports playing back a 440hz
//! tone, aka A4.

struct sound;

//! \brief Creates and initializes a new sound object
//! \return The initialized sound object
struct sound *
SoundInit();

//! \brief De-initializes and frees memory for the given sound object
//! \param[in,out] sound The initialized sound object to be cleaned and reclaimed
void
SoundDeinit(struct sound *sound);

//! \brief Start playback of A4 (440hz)
//! \param[in,out] sound Sound interface to invoke playback on
void
SoundPlay(struct sound *sound);

//! \brief Stop playback
//! \param[in,out] sound Sound interface to stop playback on
void
SoundStop(struct sound *sound);

#endif // SOUND_VERSION
