/******************************************************************************
  File: sound.h
  Date: 2019-07-07
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
          by Aaron Oman (See LICENSE)
 ******************************************************************************/
#ifndef SOUND_VERSION
#define SOUND_VERSION "0.1.0"

struct sound;

void
SoundMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

void
SoundShutdown(struct sound *sound);

struct sound *
SoundInit();

#endif // SOUND_VERSION
