BUILD_DIR = build
SRC_DIR = src
TARGET_EXEC = $(BUILD_DIR)/csvreader

CC = gcc
CFLAGS = -Wall -O2 -I$(SRC_DIR) -MMD -MP

SOURCES = \
	main.c \
	cell.c \
	lookup.c

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)
DEPS = $(SOURCES:%.c=$(BUILD_DIR)/%.d)

.PHONY: all
all: $(TARGET_EXEC)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(TARGET_EXEC): $(OBJECTS)
	$(LINK.c) $(LIBS) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(COMPILE.c) $< -o $@

$(BUILD_DIR)/%.d: $(BUILD_DIR)/%.o
	@: # Needed for GNU Make

include $(DEPS)
