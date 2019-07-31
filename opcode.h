/******************************************************************************
  File: opcode.h
  Created: 2019-06-14
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file opcode.h
//!
//! Opcode is implemented as a separate entity from the emulator system itself.
//! The CHIP-8 contains 35 opcodes and these are explicitly implemented as
//! discrete functions.
//!
//! The opcode interface provides 3 main routines for interaction:
//! 1. OpcodeFetch()
//! 2. OpcodeDecode()
//! 3. OpcodeExecute()
//!
//! OpcodeFetch() reads the instruction pointed to by the CHIP-8's program
//! counter, then increments that pointer appropriately.
//!
//! OpcodeDecode() interprets the instruction read by OpcodeFetch() and sets up
//! internal state to prepare for OpcodeExecute().
//!
//! OpcodeExecute() executes the function that represents the instruction being
//! pointed to and updates the internal state of the CHIP-8 system.

#ifndef OPCODE_VERSION
#define OPCODE_VERSION "0.1.0"

struct opcode;
struct system;

//! \brief Creates and initializes a new opcode object instance
//! \return The initialized opcode object
struct opcode *
OpcodeInit();

//! \brief De-initializes and frees memory for the given opcode object
//! \param[in,out] opcode The initialized opcode object to be cleaned and reclaimed
void
OpcodeDeinit(struct opcode *opcode);

//! \brief Reads the next instruction to be executed
//!
//! Reads the next instruction to be executed from the CHIP-8's pc and
//! increments the pc by two bytes.
//!
//! \param[in,out] opcode State representing the instruction to be executed
//! \param[in,out] system CHIP-8 system state to be read and updated
void
OpcodeFetch(struct opcode *opcode, struct system *system);

//! \brief Interprets the stored instruction and sets up state
//!
//! Sets up internal opcode state in preparation of the subsequent call to
//! OpcodeExecte().
//! This function configures internal state so OpcodeInstruction() and
//! OpcodeDescription() work; but more significantly, so we can call
//! OpcodeExecute().
//!
//! Each opcode is represented internally by a function and stored as a function
//! pointer: opcode_fn.
//!
//! \param[in,out] opcode Opcode state to be updated
//! \see CHIP-8 Opcode listing: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
void
OpcodeDecode(struct opcode *opcode);

//! \brief Executes the described opcode
//!
//! Executes the described opcode via the internally stored function.
//!
//! \param[in,out] opcode The stored opcode state
//! \param[in,out] system The CHIP-8 system state to be updated
void
OpcodeExecute(struct opcode *opcode, struct system *system);

//! \brief Returns the two-byte instruction to be executed
//!
//! Each instruction is a two-byte value representing an opcode, of which there
//! are 35 supported by the CHIP-8.
//! This function returns the two-byte value for that opcode.
//! In the CHIP-8 each instruction is stored in big-endian, but this function
//! returns the native representation after being read from the CHIP-8's memory.
//!
//! \param[in] opcode Opcode state to be read
//! \return The two-byte opcode instruction value
//! \see CHIP-8 Opcode listing: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
unsigned short
OpcodeInstruction(struct opcode *opcode);

//! \brief Returns a description of the current instruction to be executed
//!
//! \param[in] opcode Opcode state to be read
//! \param[in,out] string Pre-allocated string where description will be written
//! \param[in] maxLength Size of the pre-allocated string
//! \return 0 if the description couldn't be written, otherwise non-zero
//! \see CHIP-8 Opcode listing: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
int
OpcodeDescription(struct opcode *opcode, char *string, unsigned int maxLength);

#endif // OPCODE_VERSION
