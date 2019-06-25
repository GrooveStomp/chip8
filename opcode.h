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
OpcodeFree(void *c);

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

char *
OpcodeDescription(struct opcode *c);

#endif // OPCODE_VERSION
