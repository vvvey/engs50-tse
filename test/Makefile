# Define variables to discover all test files
TEST_FILES = $(wildcard test*.c)
TEST_OBJS = $(TEST_FILES:.c=.o)
TEST_EXECUTABLES = $(TEST_FILES:.c=)

CFLAGS= -Wall -pedantic -std=c11 -I../utils -L../lib -g
LIBS=-lutils -lcurl

all: $(TEST_EXECUTABLES)

%: %.c
	gcc $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -f $(TEST_OBJS) $(TEST_EXECUTABLES)

