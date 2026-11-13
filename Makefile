CC ?= cc

TARGET := sh311
SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
DEPENDENCIES := $(OBJECTS:.o=.d)
LEXER_TEST := tests/test_lexer
PARSER_TEST := tests/test_parser

BASE_CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
CFLAGS ?= $(BASE_CFLAGS) -O2
CPPFLAGS ?= -Isrc
LDFLAGS ?=
LDLIBS ?=

.PHONY: all debug run test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(LEXER_TEST): tests/test_lexer.c src/lexer.c src/lexer.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) \
		tests/test_lexer.c src/lexer.c \
		$(LDLIBS) -o $@

$(PARSER_TEST): tests/test_parser.c \
		src/lexer.c src/lexer.h \
		src/parser.c src/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) \
		tests/test_parser.c \
		src/lexer.c src/parser.c \
		$(LDLIBS) -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(BASE_CFLAGS) -O0 -g3 -fsanitize=address,undefined" \
		LDFLAGS="-fsanitize=address,undefined" $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET) $(LEXER_TEST) $(PARSER_TEST)
	./$(LEXER_TEST)
	./$(PARSER_TEST)
	@output="$$(printf 'printf sh311-ok\nexit\n' | ./$(TARGET))"; \
	if [ "$$output" != "sh311-ok" ]; then \
		echo "Smoke test failed: expected 'sh311-ok', got '$$output'"; \
		exit 1; \
	fi
	@echo "Smoke test passed"

clean:
	$(RM) $(TARGET) $(OBJECTS) $(DEPENDENCIES) \
		$(LEXER_TEST) $(PARSER_TEST)

-include $(DEPENDENCIES)
