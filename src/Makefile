
# WARNING: This Makefile depends on GNU Make's Makefile regeneration feature

BINDIR := ../bin
OBJDIR := ../build
OBJNAMES := \
	ast.o \
	compile_time_environment.o \
	desugar.o \
	lexer.o \
	match_identifiers.o \
	parser.o \
	parse.o \
	string_view.o \
	tester.o \
	test_set.o \
	token.o \
	typechecker.o \
	typed_ast.o \
	typesystem.o \
	interpreter/environment.o \
	interpreter/error.o \
	interpreter/eval.o \
	interpreter/execute.o \
	interpreter/garbage_collector.o \
	interpreter/value.o \
	interpreter/native.o

OBJS := $(addprefix $(OBJDIR)/,$(OBJNAMES))

LIBS := -lasan

CXXFLAGS := -std=c++14 -Wall -fsanitize=address -g
CPPFLAGS := 

all: $(BINDIR)/jasperi

test_program: $(BINDIR)/test_program

clean:
	rm $(BINDIR) -r
	rm $(OBJDIR) -r

$(BINDIR)/jasperi: $(OBJS) $(OBJDIR)/interpreter/main.o | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(BINDIR)/test_program: $(OBJS) $(OBJDIR)/tests.o | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(OBJS) $(OBJDIR)/interpreter/main.o $(OBJDIR)/tests.o: | $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/interpreter

include $(OBJDIR)/interpreter/main.d
include $(OBJDIR)/tests.d
include $(OBJS:.o=.d)
		
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

