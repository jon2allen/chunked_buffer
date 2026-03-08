# Compiler settings
CC ?= cc
CFLAGS += -Wall -Wextra -Werror -std=c11 -g -I.
LDFLAGS += 

# Library directories
STRING_BUFFER_DIR = string_buffer_lib
CHUNKED_BUFFER_DIR = chunked_buffer_lib

# Source files
CHUNKED_SRC = $(CHUNKED_BUFFER_DIR)/chunked_buffer.c
STRING_BUFFER_SRC = $(STRING_BUFFER_DIR)/string_buffer.c
TEST1_SRC = test1.c
TEST2_SRC = test2_sb.c
INTEGRATION_SRC = integration_test.c
SONNET_SRC = sonnet_search.c
PERF_SRC = performance_test.c
ITER_SRC = iter_test.c
SECURITY_SRC = test_security_risk.c

# Object files
CHUNKED_OBJ = $(CHUNKED_BUFFER_DIR)/chunked_buffer.o
STRING_BUFFER_OBJ = $(STRING_BUFFER_DIR)/string_buffer.o
TEST1_OBJ = test1.o
TEST2_OBJ = test2_sb.o
INTEGRATION_OBJ = integration_test.o
SONNET_OBJ = sonnet_search.o
PERF_OBJ = performance_test.o
ITER_OBJ = iter_test.o
SECURITY_OBJ = test_security_risk.o

# Executables
TEST1_EXEC = test_chunked_buffer
TEST2_EXEC = test_string_buffer
INTEGRATION_EXEC = integration_test
SONNET_EXEC = sonnet_search
PERF_EXEC = performance_test
ITER_EXEC = iter_test
SECURITY_EXEC = test_security_risk

# Default target - build all tests
all: $(TEST1_EXEC) $(TEST2_EXEC) $(INTEGRATION_EXEC) $(SONNET_EXEC) $(PERF_EXEC) $(ITER_EXEC) $(SECURITY_EXEC)

# Build chunked_buffer.o
$(CHUNKED_OBJ): $(CHUNKED_SRC) $(CHUNKED_BUFFER_DIR)/chunked_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build string_buffer.o
$(STRING_BUFFER_OBJ): $(STRING_BUFFER_SRC) $(STRING_BUFFER_DIR)/string_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build test1.o
$(TEST1_OBJ): $(TEST1_SRC) $(CHUNKED_BUFFER_DIR)/chunked_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build test2_sb.o
$(TEST2_OBJ): $(TEST2_SRC) $(STRING_BUFFER_DIR)/string_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build integration_test.o
$(INTEGRATION_OBJ): $(INTEGRATION_SRC) $(STRING_BUFFER_DIR)/string_buffer.h $(CHUNKED_BUFFER_DIR)/chunked_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build sonnet_search.o
$(SONNET_OBJ): $(SONNET_SRC) $(STRING_BUFFER_DIR)/string_buffer.h $(CHUNKED_BUFFER_DIR)/chunked_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build performance_test.o
$(PERF_OBJ): $(PERF_SRC) $(STRING_BUFFER_DIR)/string_buffer.h $(CHUNKED_BUFFER_DIR)/chunked_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build iter_test.o
$(ITER_OBJ): $(ITER_SRC) $(STRING_BUFFER_DIR)/string_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build test_security_risk.o
$(SECURITY_OBJ): $(SECURITY_SRC) $(STRING_BUFFER_DIR)/string_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build test_chunked_buffer executable
$(TEST1_EXEC): $(CHUNKED_OBJ) $(TEST1_OBJ)
	$(CC) $(LDFLAGS) $(CHUNKED_OBJ) $(TEST1_OBJ) -o $@

# Build test_string_buffer executable
$(TEST2_EXEC): $(STRING_BUFFER_OBJ) $(TEST2_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(TEST2_OBJ) -o $@

# Build integration_test executable
$(INTEGRATION_EXEC): $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(INTEGRATION_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(INTEGRATION_OBJ) -o $@

# Build sonnet_search executable
$(SONNET_EXEC): $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(SONNET_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(SONNET_OBJ) -o $@

# Build performance_test executable
$(PERF_EXEC): $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(PERF_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(CHUNKED_OBJ) $(PERF_OBJ) -o $@

# Build iter_test executable
$(ITER_EXEC): $(STRING_BUFFER_OBJ) $(ITER_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(ITER_OBJ) -o $@

# Build test_security_risk executable
$(SECURITY_EXEC): $(STRING_BUFFER_OBJ) $(SECURITY_OBJ)
	$(CC) $(LDFLAGS) $(STRING_BUFFER_OBJ) $(SECURITY_OBJ) -o $@

# Run all tests
test: $(TEST1_EXEC) $(TEST2_EXEC) $(INTEGRATION_EXEC) $(SONNET_EXEC) $(PERF_EXEC) $(ITER_EXEC)
	./$(TEST1_EXEC)
	./$(TEST2_EXEC)
	./$(INTEGRATION_EXEC)
	./$(SONNET_EXEC)
	./$(PERF_EXEC)
	./$(ITER_EXEC)
	./$(SECURITY_EXEC)

# Run specific test
test1: $(TEST1_EXEC)
	./$(TEST1_EXEC)

test2: $(TEST2_EXEC)
	./$(TEST2_EXEC)

integration: $(INTEGRATION_EXEC)
	./$(INTEGRATION_EXEC)

sonnet: $(SONNET_EXEC)
	./$(SONNET_EXEC)

perf: $(PERF_EXEC)
	./$(PERF_EXEC)

iter: $(ITER_EXEC)
	./$(ITER_EXEC)

security: $(SECURITY_EXEC)
	./$(SECURITY_EXEC)

# Clean up
clean:
	rm -f $(CHUNKED_OBJ) $(STRING_BUFFER_OBJ) $(TEST1_OBJ) $(TEST2_OBJ) $(INTEGRATION_OBJ) $(SONNET_OBJ) $(PERF_OBJ) $(ITER_OBJ) $(SECURITY_OBJ)
	rm -f $(TEST1_EXEC) $(TEST2_EXEC) $(INTEGRATION_EXEC) $(SONNET_EXEC) $(PERF_EXEC) $(ITER_EXEC) $(SECURITY_EXEC)

# Phony targets
.PHONY: all test test1 test2 integration sonnet perf iter security clean