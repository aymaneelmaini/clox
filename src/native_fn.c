#include "native_fn.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "object.h"

Value clock_native(int arg_count, Value* args)
{
    (void)arg_count;
    (void)args;
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value println_native(int arg_count, Value* args)
{
    for (int i = 0; i < arg_count; i++)
    {
        if (i > 0)
            printf(" ");
        print_value(args[i]);
    }
    printf("\n");
    return NIL_VAL;
}

static Value read_line_value()
{
    size_t capacity = 64;
    size_t length = 0;
    char*  buffer = malloc(capacity);
    if (buffer == NULL)
        return NIL_VAL;

    int c;
    while ((c = getchar()) != EOF && c != '\n')
    {
        if (length + 1 >= capacity)
        {
            capacity *= 2;
            char* next = realloc(buffer, capacity);
            if (next == NULL)
            {
                free(buffer);
                return NIL_VAL;
            }
            buffer = next;
        }
        buffer[length++] = (char)c;
    }

    if (c == EOF && length == 0)
    {
        free(buffer);
        return NIL_VAL;
    }

    buffer[length] = '\0';
    ObjString* string = copy_string(buffer, (int)length);
    free(buffer);
    return OBJ_VAL(string);
}

Value input_native(int arg_count, Value* args)
{
    if (arg_count > 0)
    {
        print_value(args[0]);
    }
    return read_line_value();
}

Value scan_native(int arg_count, Value* args)
{
    (void)arg_count;
    (void)args;
    return read_line_value();
}
