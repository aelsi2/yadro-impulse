BUILD_DIR = build
SRC_DIR = src
TEST_PASS_DIR = test
TEST_FAIL_DIR = test/fail
TARGET_EXEC = $(BUILD_DIR)/csvreader

CC = gcc
CFLAGS += -Wall -I$(SRC_DIR) -MMD -MP $(OPTFLAGS)
OPTFLAGS = -O2
debug: OPTFLAGS = -g -fsanitize=undefined -fsanitize=address

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.d)
TESTS_PASS = $(filter-out %.expected.csv,$(wildcard $(TEST_PASS_DIR)/*.csv))
TESTS_FAIL = $(wildcard $(TEST_FAIL_DIR)/*.csv)

.PHONY: all
all: $(TARGET_EXEC)

.PHONY: debug
debug: all

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: test
test: $(TESTS_PASS) $(TESTS_FAIL)
	@echo All tests passed!

$(TARGET_EXEC): $(OBJECTS)
	$(LINK.c) $^ -o $@

$(OBJECTS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) $< -o $@

.PHONY: $(TESTS_PASS)
$(TESTS_PASS): $(TARGET_EXEC) 
	$(TARGET_EXEC) $@ | diff - $(subst .csv,.expected.csv,$@)

.PHONY: $(TESTS_FAIL)
$(TESTS_FAIL): $(TARGET_EXEC) 
	! $(TARGET_EXEC) $@ 2> /dev/null

-include $(DEPS)
