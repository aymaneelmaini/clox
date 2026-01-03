#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
    ObjClosure* closure;
    u8*         ip;
    Value*      slots;
} CallFrame;

typedef struct
{
    CallFrame frames[FRAMES_MAX];
    int       frame_count;
    Value     stack[STACK_MAX];
    Value*    stack_top;
    Table     globals;
    Table     strings;  // for string interning just like (string pool in java)
    Obj*      objects;
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void            init_VM();
void            free_VM();
InterpretResult interpret(char* source);
void            push(Value value);
Value           pop();

#endif
