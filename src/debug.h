#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"
#include "scanner.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int  disassemble_instruction(Chunk* chunk, int offset);
void print_token(Token token);

#endif
