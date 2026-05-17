BUILD_DIR = build
SRC_DIR = src
TARGET_EXEC = $(BUILD_DIR)/csvreader

CC = gcc
CFLAGS += -Wall -I$(SRC_DIR) -MMD -MP $(OPTFLAGS)

OPTFLAGS = -O2
debug: OPTFLAGS = -g -fsanitize=undefined -fsanitize=address

SOURCES = \
	main.c \
	cell.c \
	sheet.c \
	parse.c \
	vector.c \
	lookup.c

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)
DEPS = $(SOURCES:%.c=$(BUILD_DIR)/%.d)

.PHONY: all
all: $(TARGET_EXEC)

.PHONY: debug
debug: all

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(TARGET_EXEC): $(OBJECTS)
	$(LINK.c) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) $< -o $@

-include $(DEPS)
