/*
  Chip-8 opcode
*/

struct system;

typedef void (*opcode_fn)(struct system *);

// All instructions are 2 bytes store most-significant byte first. (Big Endian)
struct opcode {
        unsigned short instruction;
        opcode_fn fn;
};

struct opcode_map {
        unsigned short key;
        opcode_fn value;
};

static struct opcode_map OpcodeMap[35];

void OpcodeInit(struct opcode *c) {
        OpcodeMap[0] = { .key = 0x0NNN, .value = __opcode_0NNN };
        OpcodeMap[1] = { .key = 0x00E0, .value = __opcode_00E0 };
        OpcodeMap[2] = { .key = 0x00EE, .value = __opcode_00EE };

        c->instruction = 0;
        c->fn = NULL;
}

// See full fn listing: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

void __opcode_0NNN(struct system *s) {
        // type: Call
        // C Pseudo:
        // Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
}

void __opcode_00E0(struct system *s) {
        // type: Display
        // C Pseudo: disp_clear()
        // Clears the screen.
}

void __opcode_00EE(struct system *s) {
        // type: Flow
        // C Psudeo: return;
        // Returns from a subroutine.
}

// Stores two-byte opcode from memory pointed to by pc into opcode c.
void OpcodeFetch(struct opcode *c, struct system *s) {
        // Fetch the first byte from memory.
        // Left shift 8 bits, padding on the right with zeroes.
        // Binary OR with the next byte from memory.
        // eg.:
        //     A200
        //  OR 00A2
        //  =======
        //     A2A2
        c->instruction = s->memory[s->pc] << 8 | s->memory[s->pc + 1];
}

// TODO: Use a proper hashing routine and do fast lookup?
//       Although, with just 35 values we're probably okay?
void OpcodeDecode(struct opcode *c) {
        for (int i = 0; i < 35; i++) {
                if (OpcodeMap[i].key == c->instruction) {
                        c->fn = OpcodeMap[i].value;
                }
        }
        printf("Unknown opcode: %#X\n", c->instruction);
}

void OpcodeExecute(struct opcode *c, struct system *s) {
        // TODO: What happens if c->fn is NULL? Consume as normal or abort?
        //       Probably abort!
        if (c->fn == NULL) {
                return;
        }

        c->fn(s);

        // Increment pc by two for every two-byte opcode we execute.
        s->pc += 2;
}
