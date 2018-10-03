# LOOM Common Library
OUTPUT   := common

COMPILER := g++
PARAMS   := -fPIC -std=c++11 -MMD -MP -Wall -Wfatal-errors -O2 -s
BUILD    := -s -shared
SRCS     := $(notdir $(basename $(shell find src -name '*.cpp')))
INCL     := -Iinclude

OBJS-LINUX  := $(addsuffix .o, $(addprefix ./obj/linux/, $(SRCS)))
OBJS-CYGWIN := $(addsuffix .o, $(addprefix ./obj/cygwin/, $(SRCS)))
DEPS-LINUX  := $(OBJS-LINUX:.o=.d)
DEPS-CYGWIN := $(OBJS-CYGWIN:.o=.d)

build-linux: BINDIR := ../bin/linux
build-linux: LIBDIR := ../lib/linux
build-linux: LIB := -lpthread
build-linux: need-obj-dir $(OBJS-LINUX)
	mkdir -p $(LIBDIR)
	mkdir -p $(BINDIR)
	$(COMPILER) $(BUILD) -Wl,-rpath=$(LIBDIR) $(OBJS-LINUX) $(LIB) -o $(LIBDIR)/lib$(OUTPUT).so

build-cygwin: BINDIR := ../bin/cygwin
build-cygwin: LIBDIR := ../lib/cygwin
build-cygwin: LIB := -lpthread
build-cygwin: need-obj-dir $(OBJS-CYGWIN)
	mkdir -p $(LIBDIR)
	mkdir -p $(BINDIR)
	$(COMPILER) $(BUILD) $(OBJS-CYGWIN) $(LIB) -o $(BINDIR)/lib$(OUTPUT).dll -Wl,--out-implib,$(LIBDIR)/lib$(OUTPUT).a

clean:
	rm -rf obj/

need-obj-dir:
	mkdir -p obj/linux
	mkdir -p obj/cygwin

obj/linux/%.o: src/%.cpp
	$(COMPILER) -c $(PARAMS) $(INCL) $< -o $@
obj/cygwin/%.o: src/%.cpp
	$(COMPILER) -c $(PARAMS) $(INCL) $< -o $@ -D_CYGWIN_BUILD_ -D_LOOM_EXPORT_

-include $(DEPS-LINUX)
-include $(DEPS-CYGWIN)
