#ifndef SOUND_H
#define SOUND_H

struct sound;

void
SoundMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

void
SoundShutdown(struct sound *sound);

struct sound *
SoundInit();

#endif // SOUND_H
