
# WARNING: This Makefile depends on GNU Make's Makefile regeneration feature

BINDIR := ../bin
OBJDIR := ../build
OBJNAMES := \
	ast.o \
	captures.o \
	environment.o \
	error.o \
	eval.o \
	execute.o \
	garbage_collector.o \
	lexer.o \
	match_identifiers.o \
	parser.o \
	parse.o \
	string_view.o \
	tester.o \
	token.o \
	typechecker.o \
	typed_ast.o \
	typesystem.o \
	value.o \
	native.o

OBJS := $(addprefix $(OBJDIR)/,$(OBJNAMES))

LIBS := -lasan

CXXFLAGS := -std=c++14 -Wall -fsanitize=address -g
CPPFLAGS := 

all: $(BINDIR)/iseeaparse

test_program: $(BINDIR)/test_program

clean:
	rm $(BINDIR) -r
	rm $(OBJDIR) -r

$(BINDIR)/iseeaparse: $(OBJS) $(OBJDIR)/main.o | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(BINDIR)/test_program: $(OBJS) $(OBJDIR)/tests.o | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJS) $(OBJDIR)/main.o $(OBJDIR)/tests.o: | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

include $(OBJDIR)/main.d
include $(OBJDIR)/tests.d
include $(OBJS:.o=.d)
		
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

