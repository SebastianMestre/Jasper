
SOURCE_DIR := src
BIN_DIR    := bin

BUILD_BASE_DIR  := build

# modules
INTERPRETER := jasperi
TEST       := run_tests

CXXFLAGS := -std=c++14 -Wall
LIBS :=

COMMON_DIR := .
COMMON_TARGETS := \
	algorithms/tarjan_solver \
	algorithms/unification \
	utils/block_allocator \
	utils/interned_string \
	utils/span \
	utils/string_set \
	utils/string_view \
	ast \
	compile_time_environment \
	compute_offsets \
	ct_eval \
	desugar \
	error_report \
	lexer \
	match_identifiers \
	metacheck \
	parse \
	parser \
	token \
	typecheck \
	typechecker \
	typed_ast \
	typesystem

INTERPRETER_DIR := interpreter
INTERPRETER_ENTRY := main
INTERPRETER_TARGETS := \
	environment \
	error \
	eval \
	execute \
	garbage_collector \
	gc_ptr \
	native \
	utils \
	value

TEST_DIR := test
TEST_ENTRY := main
TEST_TARGETS := \
	test_set \
	tester

ifeq ($(MODE),debug)
  CXXFLAGS += -g -fsanitize=address
  LIBS += -lasan
  BUILD_DIR := $(BUILD_BASE_DIR)/debug
else ifeq ($(MODE),tuning)
  CXXFLAGS += -O2 -g -fno-omit-frame-pointer
  BUILD_DIR := $(BUILD_BASE_DIR)/tuning
else ifeq ($(MODE),dev)
  CXXFLAGS += -O0
  BUILD_DIR := $(BUILD_BASE_DIR)/dev
else
  CXXFLAGS += -O3
  BUILD_DIR := $(BUILD_BASE_DIR)/release
endif

COMMON_OBJECTS      := $(COMMON_TARGETS:%=$(BUILD_DIR)/$(COMMON_DIR)/%.o)
INTERPRETER_OBJECTS := $(INTERPRETER_TARGETS:%=$(BUILD_DIR)/$(INTERPRETER_DIR)/%.o)
TEST_OBJECTS        := $(TEST_TARGETS:%=$(BUILD_DIR)/$(TEST_DIR)/%.o)

INTERPRETER_ENTRY_OBJECT := $(BUILD_DIR)/$(INTERPRETER_DIR)/$(INTERPRETER_ENTRY).o
TEST_ENTRY_OBJECT        := $(BUILD_DIR)/$(TEST_DIR)/$(TEST_ENTRY).o

ALL_OBJECTS := \
	$(COMMON_OBJECTS) \
	$(INTERPRETER_OBJECTS) $(INTERPRETER_ENTRY_OBJECT) \
	$(TEST_OBJECTS) $(TEST_ENTRY_OBJECT)

DEPS := $(ALL_OBJECTS:%.o=%.d)

TEST_BIN := $(BIN_DIR)/$(TEST)
INTERPRETER_BIN := $(BIN_DIR)/$(INTERPRETER)

# UTILS

SHOW_COMMAND := @printf "%-15s%s\n"
SHOW_CXX := $(SHOW_COMMAND) "[ $(CXX) ]"
SHOW_DEPS := $(SHOW_COMMAND) "[ INCLUDE ]"

# TARGET DEFINITIONS
all: $(INTERPRETER_BIN)
.PHONY: all

clean:
	rm -r $(BUILD_BASE_DIR)
.PHONY: clean

interpreter: $(INTERPRETER_BIN)
.PHONY: interpreter

tests: $(TEST_BIN)
.PHONY: tests

$(TEST_BIN): $(TEST_ENTRY_OBJECT) $(TEST_OBJECTS) $(INTERPRETER_OBJECTS) $(COMMON_OBJECTS)
$(INTERPRETER_BIN): $(INTERPRETER_ENTRY_OBJECT) $(INTERPRETER_OBJECTS) $(COMMON_OBJECTS)

include $(DEPS)

# RULES

$(TEST_BIN) $(INTERPRETER_BIN):
	$(SHOW_CXX) $@
	@mkdir -p $(dir $@)
	@$(CXX) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(SHOW_CXX) $@
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	@#$(SHOW_DEPS) $@
	@mkdir -p $(dir $@)
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($(*F)\)\.o[ :]*,$(BUILD_DIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

