# LOOM YARN Assembler
OUTPUT   := loomcompile

COMPILER := g++
PARAMS   := -std=c++11 -MMD -MP -Wall -Wfatal-errors -O2 -s
BUILD    := -s
SRCS     := $(notdir $(basename $(shell find src -name '*.cpp')))
INCL     := -I../common/include -Iinclude

OBJS-LINUX  := $(addsuffix .o, $(addprefix ./obj/linux/, $(SRCS)))
OBJS-CYGWIN := $(addsuffix .o, $(addprefix ./obj/cygwin/, $(SRCS)))
DEPS-LINUX  := $(OBJS-LINUX:.o=.d)
DEPS-CYGWIN := $(OBJS-CYGWIN:.o=.d)

build-linux: BINDIR := ../bin/linux
build-linux: LIBDIR := ../lib/linux
build-linux: LIB := -L$(LIBDIR) -lcommon
build-linux: need-obj-dir $(OBJS-LINUX)
	mkdir -p $(LIBDIR)
	mkdir -p $(BINDIR)
	$(COMPILER) $(BUILD) -Wl,-rpath=$(LIBDIR) $(OBJS-LINUX) $(LIB) -o $(BINDIR)/$(OUTPUT)

build-cygwin: BINDIR := ../bin/cygwin
build-cygwin: LIBDIR := ../lib/cygwin
build-cygwin: LIB := -L$(LIBDIR) -lcommon
build-cygwin: need-obj-dir $(OBJS-CYGWIN)
	mkdir -p $(LIBDIR)
	mkdir -p $(BINDIR)
	$(COMPILER) $(BUILD) -Wl,-rpath=$(LIBDIR) $(OBJS-CYGWIN) $(LIB) -o $(BINDIR)/$(OUTPUT).exe

clean:
	rm -rf obj/

need-obj-dir:
	mkdir -p obj/linux
	mkdir -p obj/cygwin

obj/linux/%.o: src/%.cpp
	$(COMPILER) -c $(PARAMS) $(INCL) $< -o $@
obj/cygwin/%.o: src/%.cpp
	$(COMPILER) -c $(PARAMS) $(INCL) $< -o $@

-include $(DEPS-LINUX)
-include $(DEPS-CYGWIN)
