/******************************************************************************
  File: opcode.h
  Created: 2019-06-14
  Updated: 2019-07-16
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef OPCODE_VERSION
#define OPCODE_VERSION "0.1.0"

struct opcode;
struct system;

typedef void (*opcode_fn)(struct opcode *c, struct system *);

void
OpcodeMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

struct opcode *
OpcodeInit();

void
OpcodeDeinit(struct opcode *c);

// Stores two-byte opcode from memory pointed to by pc into opcode c.
void
OpcodeFetch(struct opcode *c, struct system *s);

void
OpcodeDecode(struct opcode *c, struct system *s);

void
OpcodeExecute(struct opcode *c, struct system *s);

void
OpcodeDebug(struct opcode *c);

unsigned short
OpcodeInstruction(struct opcode *c);

// Returns non-zero if successfully written.
int
OpcodeDescription(struct opcode *c, char *str, unsigned int maxLen);

#endif // OPCODE_VERSION
