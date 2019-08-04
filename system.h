/******************************************************************************
  File: system.h
  Created: 2019-06-13
  Updated: 2019-08-04
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
//! \file system.h
//!
//! This is the core system emulation package.
//!
//! Generally speaking, the emulation occurs on the main thread with other
//! subsystems operating in separate threads.  Threadsafe synchronization
//! primitives are used in system methods to allow threadsafe data access and
//! manipulation.
//!
//! There are two small logical subsystems of this package to handle tricky
//! state management:
//! - WFK (aka Wait For Key)
//! - Debug
//!
//! These logical subsystems are interesting because they break the normal
//! opcode:fetch -> opcode:decode -> opcode:execute flow of the emulation.
//!
//! They should be thought of as logical subsystems and their methods are
//! grouped syntactically together:
//! - WFK methods are all prefixed with: SystemWFK
//! - Debug methods are all prefixed with: SystemDebug

#ifndef SYSTEM_VERSION
//! include guard
#define SYSTEM_VERSION "0.1.0"

struct system_private;

struct system {
        //! 4k System memory map:
        //! 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
        //! 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
        //! 0x200-0xFFF - Program ROM and work RAM
        unsigned char *memory;

        //! CPU registers: The Chip 8 has 15 8-bit general purpose registers
        //! named V0,V1 up to VE. The 16th register is used for the ‘carry
        //! flag’. Eight bits is one byte so we can use an unsigned char for this
        //! purpose:
        unsigned char v[16];

        unsigned short i; //!< index register
        unsigned short pc; //!< Program counter can be [0x000..0xFFF]

        //! The graphics of the Chip 8 are black and white and the screen has a
        //! total of 2048 pixels (64 x 32). This can easily be implemented using
        //! an array that hold the pixel state (1 or 0)
        unsigned char *gfx;

        //! The stack allows storing up to 16 addresses. Each address in the
        //! stack is the location of a caller, so the stack works like function
        //! calls.
        unsigned short stack[16]; //!< Function call stack
        unsigned short sp; //!< Stack pointer, indexes head element of stack

        //! The Chip 8 has a HEX based keypad (0x0-0xF), you can use an
        //! array to store the current state of the key.
        unsigned char key[16]; //!< Keyboard state (key pressed or not)

        unsigned short fontp; //!< Pointer to font sprits in CHIP-8 memory

        struct system_private *prv; //!< Unexported implementation data
};

//! \brief Creates and initializes a new system object instance
//! \param[in] isDebugEnabled whether to run with the integrated debugging UI
//! \return The initialized system object
struct system *
SystemInit(int isDebugEnabled);

//! \brief De-initializes and frees memory for the given system object
//! \param[in,out] system The initialized system object to be cleaned and reclaimed
void
SystemDeinit(struct system *system);

//! \brief Increments the system PC to point to the next instruction
//!
//! The CHIP-8 uses 16-bit instructions, but stores that data big-endian.
//! As a result, we read instructions as two 8-bit bytes in reverse order and
//! increment the PC by two bytes.
//!
//! \param[in,out] system system state to be updated
void
SystemIncrementPC(struct system *system);

//! \brief Gets the CHIP-8 memory address of the desired font sprite
//! \param[in] system system state to be read
//! \param[in] index Which font sprite to get (0-F)
//! \return address in CHIP-8 memory for the sprite
unsigned short
SystemFontSprite(struct system *system, unsigned int index);

//! \brief Copies ROM into the CHIP-8's memory
//! \param[in,out] system system state memory to be updated
//! \param[in] rom program ROM to be loaded into CHIP-8
//! \param[in] size  size of program ROM to be loaded
//! \return non-zero if program could be loaded, otherwise 0
int
SystemLoadProgram(struct system *system, unsigned char *rom, unsigned int size);

//! \brief Sets stack[sp] to pc and increments sp
//! \param[in,out] system system state to be updated
void
SystemStackPush(struct system *system);

//! \brief Sets pc to stack[sp] and decrements sp
//! \param[in,out] system system state to be updated
void
SystemStackPop(struct system *system);

//! \brief Atomically locks the CHIP-8's graphics memory
//!
//! Threadsafe.
//!
//! Lock graphics memory so a separate thread can use it for rendering.
//!
//! \param[in,out] system system state to be updated
//! \return 0 if the lock is held otherwise non-zero
int
SystemGfxLock(struct system *system);

//! \brief Atomically unlocks the CHIP-8's graphics memory
//!
//! Threadsafe.
//!
//! We need to lock graphics memory for rendering via a separate thread, this
//! routine just unlocks it after that has been done.
//!
//! \param[in,out] system system state to be updated.
//! \return 0 if lock has been released otherwise non-zero
int
SystemGfxUnlock(struct system *system);

//! \brief resets CHIP-8's video memory to zeroes
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated.
void
SystemClearScreen(struct system *system);

//! \brief Draws a sprite in video memory
//!
//! Threadsafe.
//!
//! Display: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
//! and a height of N pixels. Each row of 8 pixels is read as bit-coded starting
//! from memory location I; I value doesn’t change after the execution of this
//! instruction. As described above, VF is set to 1 if any screen pixels are
//! flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t
//! happen.
//! I'm assuming (VX, VY) is the lower-left corner of the sprint, not the center.
//!
//! \param[in,out] system system state to be updated
//! \param[in] x Which register holds the x-coordinate
//! \param[in] y Which register holds the y-coordinate
//! \param[in] height Height in pixels of the sprite to be drawn
void
SystemDrawSprite(struct system *system, unsigned int x, unsigned int y, unsigned int height);

//! \brief Tell the system to wait for a keypress
//!
//! Threadsafe.
//!
//! When a keypress occurs, it will be one of the 16 hex keys used by the
//! CHIP-8, indexed [0..F]
//! Store that index in the specified register when it occurs.
//!
//! \param[in,out] system system state to be updated
//! \param[in] reg which register to store the pressed hex key index in
//!
//! \see SystemWFKWaiting()
//! \see SystemWFKOccurred()
//! \see SystemWFKChanged()
//! \see SystemWFKStop()
void
SystemWFKSet(struct system *system, unsigned char reg);

//! \brief Is the system waiting for a keypress?
//!
//! Threadsafe.
//!
//! When the system is waiting for a keypress, it does not fetch, decode and
//! execute instructions normally. However, timers do continue to decrement.
//!
//! \param[in,out] system system state to be updated
//! \return 0 if the system is waiting for a keypress, otherwise non-zero
//!
//! \see SystemWFKSet()
//! \see SystemWFKOccurred()
//! \see SystemWFKChanged()
//! \see SystemWFKStop()
int
SystemWFKWaiting(struct system *system);

//! \brief Tell the system that a keypress occured, so stop waiting
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated
//! \param[in] key which of the keys [0..F] was pressed
//!
//! \see SystemWFKSet()
//! \see SystemWFKWaiting()
//! \see SystemWFKChanged()
//! \see SystemWFKStop()
void
SystemWFKOccurred(struct system *system, unsigned char key);

//! \brief Has the system just transitioned out of a WFK state?
//!
//! Threadsafe.
//!
//! We can signal to the system that a keypress occurred via
//! SystemWFKOccurred(), but we still want to query afterward whether we were
//! previously in a WFK state.
//!
//! \param[in,out] system system state to be updated
//! \return 0 if the system has not transitioned, otherwise non-zero
//!
//! \see SystemWFKSet()
//! \see SystemWFKWaiting()
//! \see SystemWFKOccurred()
//! \see SystemWFKStop()
int
SystemWFKChanged(struct system *system);

//! \brief Tell the system to stop waiting for a keypress
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated
//!
//! \see SystemWFKSet()
//! \see SystemWFKWaiting()
//! \see SystemWFKOccurred()
//! \see SystemWFKChanged()
void
SystemWFKStop(struct system *system);

//! \brief Decrements both the sound and delay timers
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated
void
SystemDecrementTimers(struct system *system);

//! \brief Returns the value of the delay timer
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated
//! \return value of delay timer
int
SystemDelayTimer(struct system *system);

//! \brief Returns the value of the sound timer
//!
//! Threadsafe.
//!
//! \param[in,out] system system state to be updated
//! \return value of sound timer
int
SystemSoundTimer(struct system *system);

//! \brief Sets the value of the sound and delay timers
//!
//! Threadsafe.
//!
//! It's not always desirable to set both timers at once, so use -1 _NOT_ to set
//! a timer.
//!
//! \param[in,out] system system state to be updated
//! \param[in] delayTimer value to set delayTimer to, -1 to ignore
//! \param[in] soundTimer value to set soundTimer to, -1 to ignore
void
SystemSetTimers(struct system *system, int delayTimer, int soundTimer);

//! \brief Has the sound timer reached zero after being set?
//!
//! Threadsafe.
//! Called by SoundThread()
//! \param[in,out] system system state to be read
//! \return 0 if sound has not been triggered, otherwise non-zero
int
SystemSoundTriggered(struct system *system);

//! \brief Tell the system that sound playback should begin
//!
//! Threadsafe.
//! Called when the sound timer reaches zero after being set.
//! Called by SoundThread() when sound playback begins, disabling the trigger.
//!
//! \param[in,out] system system state to be updated
//! \param[in] triggeredStatus 0 if not triggered, otherwise non-zero
//! \see soundthread.c
void
SystemSoundSetTrigger(struct system *system, int triggeredStatus);

//! \brief Returns whether the system is in a "Ready to Shutdown" state.
//!
//! Threadsafe.
//! \param[in,out] system system state to be read
//! \return 1 if ready to quit, otherwise 0
int
SystemShouldQuit(struct system *system);

//! \brief Signal to the system that shutdown should begin.
//!
//! Threadsafe.
//! \param[in] system system state to be updated
void
SystemSignalQuit(struct system *system);

//! \brief Has the embedded graphical debugger been enabled?
//!
//! Threadsafe.
//!
//! \param[in] system system state to be read
//! \return 0 if debugging is _NOT_ enabled, otherwise non-zero
//!
//! \see SystemDebugSetEnabled()
//! \see SystemDebugShouldFetchAndDecode()
//! \see SystemDebugShouldExecute()
//! \see SystemDebugSetFetchAndDecode()
//! \see SystemDebugSetExecute()
//! \see main()
//! \see TimerThread()
int
SystemDebugIsEnabled(struct system *system);

//! \brief Tell the system to enable or disable the embedded graphical debugger
//!
//! Threadsafe.
//!
//! \param[in] system system state to be udpated
//! \param[in,out] onOrOff 0 to disable debugger, or non-zero to enable
//!
//! \see SystemDebugIsEnabled()
//! \see SystemDebugShouldFetchAndDecode()
//! \see SystemDebugShouldExecute()
//! \see SystemDebugSetFetchAndDecode()
//! \see SystemDebugSetExecute()
//! \see UIWidgets()
void
SystemDebugSetEnabled(struct system *system, int onOrOff);

//! \brief Should the system perform opcode fetch and decode?
//!
//! Threadsafe.
//!
//! When the debugging UI is enabled, we need to stop the normal flow of:
//! opcode:fetch -> opcode:decode -> opcode:execute.
//!
//! This query tells us whether it is time to do one step through that flow or
//! not.
//!
//! \param[in] system system state to be read
//! \return 0 if fetch and decode should _not_ happen, otherwise non-zero
//!
//! \see SystemDebugIsEnabled()
//! \see SystemDebugSetEnabled()
//! \see SystemDebugShouldExecute()
//! \see SystemDebugSetFetchAndDecode()
//! \see SystemDebugSetExecute()
//! \see main()
int
SystemDebugShouldFetchAndDecode(struct system *system);

//! \brief Should the system perform opcode execution?
//!
//! Threadsafe.
//!
//! When the debugging UI is enabled, we need to stop the normal flow of:
//! opcode:fetch -> opcode:decode -> opcode:execute.
//!
//! This query tells us whether opcode execution should occur this "tick."
//!
//! After doing a fetch and decode, we usually want to inspect the system state
//! in the debugger, so we don't want to automatically perform the execute. We
//! need a pause between fetch+decode and then execute.
//!
//! \param[in] system system state to be read
//! \return 0 if execute should _not_ happen, otherwise non-zero
//!
//! \see SystemDebugIsEnabled()
//! \see SystemDebugSetEnabled()
//! \see SystemDebugShouldFetchAndDecode()
//! \see SystemDebugSetFetchAndDecode()
//! \see SystemDebugSetExecute()
//! \see main()
int
SystemDebugShouldExecute(struct system *system);

//! \brief Tell the system whether fetch and decode should occur this tick or not.
//!
//! Threadsafe.
//!
//! When the debugging UI is enabled, we need to stop the normal flow of:
//! opcode:fetch -> opcode:decode -> opcode:execute.
//!
//! \param[in,out] system system state to be updated
//! \param[in] onOrOff 0 to indicate fetch and decode should _not_ occur, otherwise non-zero
//!
//! \see SystemDebugIsEnabled()
//! \see SystemDebugSetEnabled()
//! \see SystemDebugShouldFetchAndDecode()
//! \see SystemDebugShouldExecute()
//! \see SystemDebugSetExecute()
//! \see main()
//! \see UIWidgets()
void
SystemDebugSetFetchAndDecode(struct system *system, int onOrOff);

//! \brief Tell the system whether execute should occur this tick or not.
//!
//! Threadsafe.
//!
//! When the debugging UI is enabled, we need to stop the normal flow of:
//! opcode:fetch -> opcode:decode -> opcode:execute.
//!
//! \param[in,out] system system state to be updated
//! \param[in] onOrOff 0 to indicate execute should _not_ occur, otherwise non-zero
//!
//! \see SystemDebugIsEnabled()
//! \see SystemDebugSetEnabled()
//! \see SystemDebugShouldFetchAndDecode()
//! \see SystemDebugShouldExecute()
//! \see SystemDebugSetFetchAndDecode()
//! \see main()
//! \see UIWidgets()
void
SystemDebugSetExecute(struct system *system, int onOrOff);

//! \brief Is the key at the given index pressed?
//!
//! Threadsafe.
//!
//! Called by opcode.c in FnEX9E() and FnEXA1().
//!
//! Called by ui.c in UIWidgets().
//!
//! \param[in] system system state to be read
//! \param[in] key Index of the hex key state being queried. [0..F]
//! \return 0 if key is _not_ pressed, otherwise non-zero
//!
//! \see SystemKeySetPressed()
int
SystemKeyIsPressed(struct system *system, int key);

//! \brief Tell the system whether the given key has been pressed or not.
//!
//! Threadsafe.
//!
//! Called via InputCheck() in input.c.
//!
//! \param[in,out] system system state to be updated
//! \param[in] key Index of hex key that has been pressed [0..F]
//! \param[in] pressed 0 if not pressed, otherwise non-zero
//!
//! \see SystemKeyIsPressed()
void
SystemKeySetPressed(struct system *system, int key, int pressed);

#endif // SYSTEM_VERSION
