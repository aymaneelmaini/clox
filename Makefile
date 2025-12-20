CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g
CTEST_FLAGS = -std=c99 -g
LDFLAGS = -lcriterion

SOURCES = src/scanner.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/value.c src/vm.c src/object.c src/table.c
TEST_SOURCES = tests/scanner_test.c tests/compiler_test.c

all: clox

clox: src/main.c $(SOURCES)
	$(CC) $(CFLAGS) -o clox src/main.c $(SOURCES) -I.

run: clox
	./clox

test: $(TEST_SOURCES) $(SOURCES)
	$(CC) $(CTEST_FLAGS) -o test_runner $(TEST_SOURCES) $(SOURCES) -I. $(LDFLAGS)
	./test_runner $(if $(ARGS),--filter "$(ARGS)")

testf: $(TEST_SOURCES) $(SOURCES)
	$(CC) $(CTEST_FLAGS) -o test_runner $(TEST_SOURCES) $(SOURCES) -I. $(LDFLAGS)
	./test_runner --fail-fast

clean:
	rm -f test_runner clox
