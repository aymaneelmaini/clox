#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void repl()
{
    char line[1024];

    for (;;)
    {
        printf(">  ");
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }
        interpret(line);
    }
}

static char* read_file(const char* path)
{
    FILE* fd = fopen(path, "rb");
    if (fd == NULL)
    {
        fprintf(stderr, "Could not open file %s \n", path);
        exit(74);
    }

    fseek(fd, 0L, SEEK_END);
    size_t fileSize = ftell(fd);
    rewind(fd);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read %s \n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fd);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file %s \n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(fd);

    return buffer;
}

static void run_file(const char* path)
{
    char*           source = read_file(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int main(int argc, const char* argv[])
{
    init_VM();

    if (argc == 1)
        repl();
    else if (argc == 2)
        run_file(argv[1]);
    else
    {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    free_VM();
    return 0;
}
