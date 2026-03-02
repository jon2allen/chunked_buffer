# Test Suite Makefile

# Compiler settings
CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -I..
LDFLAGS = 

# Test programs
TEST_PROGRAMS = \
	test_stringbuffer_security \
	test_chunkedbuffer_security

# Default target - build and run all tests
all: $(TEST_PROGRAMS)
	@echo "Running all security tests..."
	./test_stringbuffer_security
	./test_chunkedbuffer_security

# Build test_stringbuffer_security
test_stringbuffer_security: test_stringbuffer_security.c
	$(CC) $(CFLAGS) ../string_buffer_lib/string_buffer.c $< -o $@ $(LDFLAGS)

# Build test_chunkedbuffer_security
test_chunkedbuffer_security: test_chunkedbuffer_security.c
	$(CC) $(CFLAGS) ../chunked_buffer_lib/chunked_buffer.c $< -o $@ $(LDFLAGS)

# Run specific tests
run_stringbuffer: test_stringbuffer_security
	./test_stringbuffer_security

run_chunkedbuffer: test_chunkedbuffer_security
	./test_chunkedbuffer_security

# Clean up
clean:
	rm -f $(TEST_PROGRAMS) *~ *.o

# Phony targets
.PHONY: all clean run_stringbuffer run_chunkedbuffer