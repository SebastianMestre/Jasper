
# WARNING: This Makefile depends on GNU Make's Makefile regeneration feature

BINDIR := ../bin
OBJDIR := ../build
OBJNAMES := \
	main.o \
	lexer.o \
	parser.o \
	ast.o \
	eval.o \
	runtime.o \
	garbage_collector.o \
	error.o \
	environment.o

OBJS := $(addprefix $(OBJDIR)/,$(OBJNAMES))

LIBS := -lasan

CXXFLAGS := -std=c++17 -Wall -fsanitize=address -g
CPPFLAGS := 

all: $(BINDIR)/iseeaparse

clean:
	rm $(BINDIR) -r
	rm $(OBJDIR) -r

$(BINDIR)/iseeaparse: $(OBJS) | $(BINDIR)
	$(CXX) -o $@ $^ $(LIBS)


$(BINDIR):
	mkdir $(BINDIR)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

include $(OBJS:.o=.d)
		
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

