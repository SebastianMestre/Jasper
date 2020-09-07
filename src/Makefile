
# WARNING: This Makefile depends on GNU Make's Makefile regeneration feature

BINDIR := ../bin
OBJDIR := ../build

OBJNAMES := \
	ast.o \
	compile_time_environment.o \
	desugar.o \
	error_report.o \
	lexer.o \
	match_identifiers.o \
	parser.o \
	parse.o \
	string_view.o \
	tarjan_solver.o \
	token.o \
	typecheck.o \
	typechecker.o \
	typed_ast.o \
	typesystem.o

# ITPR stands for "interpreter"
ITPR_OBJNAMES := \
	interpreter/environment.o \
	interpreter/error.o \
	interpreter/eval.o \
	interpreter/execute.o \
	interpreter/garbage_collector.o \
	interpreter/gc_ptr.o \
	interpreter/value.o \
	interpreter/native.o \
	interpreter/utils.o

TEST_OBJNAMES := \
	test/tester.o \
	test/test_set.o

ITPR_ENTRY_POINT := $(OBJDIR)/interpreter/main.o
TEST_ENTRY_POINT := $(OBJDIR)/test/main.o

ENTRY_POINTS := $(ITPR_ENTRY_POINT) $(TEST_ENTRY_POINT)
OBJS := $(addprefix $(OBJDIR)/,$(OBJNAMES))
TEST_OBJS := $(addprefix $(OBJDIR)/,$(TEST_OBJNAMES))
ITPR_OBJS := $(addprefix $(OBJDIR)/,$(ITPR_OBJNAMES))
ALL_OBJS := $(OBJS) $(ITPR_OBJS) $(TEST_OBJS)

LIBS := -lasan

CXXFLAGS := -std=c++14 -Wall -fsanitize=address -g

# maybe we should build the tests as well?
all: interpreter
interpreter: $(BINDIR)/jasperi
test_program: $(BINDIR)/test_program

clean:
	rm $(BINDIR) -r
	rm $(OBJDIR) -r

$(BINDIR)/jasperi: $(OBJS) $(ITPR_OBJS) $(ITPR_ENTRY_POINT) | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(BINDIR)/test_program: $(OBJS) $(ITPR_OBJS) $(TEST_OBJS) $(TEST_ENTRY_POINT) | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(ALL_OBJS) $(ENTRY_POINTS): | $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/interpreter
	mkdir $(OBJDIR)/test

include $(ENTRY_POINTS:.o=.d)
include $(ALL_OBJS:.o=.d)
		
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($(*F)\)\.o[ :]*,$(OBJDIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

