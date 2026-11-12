CC ?= cc

TARGET := sh311
SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

BASE_CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
CFLAGS ?= $(BASE_CFLAGS) -O2
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

.PHONY: all debug run test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(BASE_CFLAGS) -O0 -g3 -fsanitize=address,undefined" \
		LDFLAGS="-fsanitize=address,undefined" $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@output="$$(printf 'printf sh311-ok\nexit\n' | ./$(TARGET))"; \
	if [ "$$output" != "sh311-ok" ]; then \
		echo "Smoke test failed: expected 'sh311-ok', got '$$output'"; \
		exit 1; \
	fi
	@echo "Smoke test passed"

clean:
	$(RM) $(TARGET) $(OBJECTS) $(DEPENDENCIES)

-include $(DEPENDENCIES)
