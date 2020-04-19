
BINDIR := ../bin
OBJDIR := ../build
OBJNAMES := \
	main.o \
	lexer.o \
	parser.o \
	ast.o \
	runtime.o \
	garbage_collector.o

OBJS := $(addprefix $(OBJDIR)/,$(OBJNAMES))

LIBS := -lasan

CXXFLAGS := -std=c++17 -Wall -fsanitize=address -g

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
		
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.hpp:
	touch $@

$(OBJDIR)/main.o: main.cpp lexer.hpp parser.hpp ast.hpp | $(OBJDIR)

$(OBJDIR)/lexer.o: lexer.cpp lexer.hpp | $(OBJDIR)

$(OBJDIR)/parser.o: parser.cpp parser.hpp lexer.hpp ast.hpp | $(OBJDIR)

$(OBJDIR)/ast.o: ast.cpp ast.hpp types.hpp | $(OBJDIR)

$(OBJDIR)/runtime.o: runtime.cpp runtime.hpp types.hpp | $(OBJDIR)

$(OBJDIR)/garbage_collector.o: garbage_collector.cpp garbage_collector.hpp runtime.hpp | $(OBJDIR)

