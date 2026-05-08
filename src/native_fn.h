#ifndef clox_native_fn_c
#define clox_native_fn_c

#include "value.h"

Value clock_native(int arg_count, Value* args);
Value println_native(int arg_count, Value* args);
Value input_native(int arg_count, Value* args);
Value scan_native(int arg_count, Value* args);

#endif
