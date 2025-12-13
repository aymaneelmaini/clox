#include "../src/compiler.h"
#include "../src/debug.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <stdint.h>
#include <stdio.h>

static void assert_bytecode(Chunk* chunk, const uint8_t* expected, int count);
static void assert_constants(Chunk* chunk, const double* expected, int count);

Test(compiler, should_compile_expressions)
{
    Chunk chunk;
    init_chunk(&chunk);

    char* source = "3 + 2";

    bool result = compile(source, &chunk);

    uint8_t expected_bytes[] = {OP_CONSTANT, 0,      OP_CONSTANT,
                                1,           OP_ADD, OP_RETURN};
    double  constants[] = {3, 2};

    assert_bytecode(&chunk, expected_bytes, 6);
    assert_constants(&chunk, constants, 2);
}

Test(compiler, should_start_with_multiplication)
{
    Chunk chunk;
    init_chunk(&chunk);

    char* source = "3 + 2 * 9";

    bool result = compile(source, &chunk);

    uint8_t expected_bytes[] = {OP_CONSTANT, 0,           OP_CONSTANT,
                                1,           OP_CONSTANT, 2,
                                OP_MULTIPLY, OP_ADD,      OP_RETURN};
    double  expected_constants[] = {3, 2, 9};

    assert_bytecode(&chunk, expected_bytes, 9);
    assert_constants(&chunk, expected_constants, 3);
}

Test(compiler, should_respect_grouping)
{
    Chunk chunk;
    init_chunk(&chunk);

    char* source = "3 + 2 * (9 + 3)";

    bool result = compile(source, &chunk);

    uint8_t expected_bytes[] = {
        OP_CONSTANT, 0, OP_CONSTANT, 1,           OP_CONSTANT, 2,
        OP_CONSTANT, 3, OP_ADD,      OP_MULTIPLY, OP_ADD,      OP_RETURN};

    double expected_constants[] = {3, 2, 9, 3};

    assert_bytecode(&chunk, expected_bytes, 12);
    assert_constants(&chunk, expected_constants, 4);
}

static void assert_bytecode(Chunk* chunk, const uint8_t* expected, int count)
{
    cr_assert_eq(chunk->count, count);
    for (int i = 0; i < count; i++)
    {
        cr_assert_eq(chunk->code[i], expected[i],
                     "code[%d]: expected %d, got %d", i, expected[i],
                     chunk->code[i]);
    }
}

static void assert_constants(Chunk* chunk, const double* expected, int count)
{
    for (int i = 0; i < count; i++)
    {
        cr_assert_eq(chunk->constants.values[i], expected[i],
                     "constant[%d]: expected %f, got %f", i, expected[i],
                     chunk->constants.values[i]);
    }
}
