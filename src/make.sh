#!/usr/bin/env bash

[ -f Makefile ] && make clean && rm -f Makefile* cversion.h

touch cversion.h

ttcn3_makefilegen -e test_suite \
		  -w \
		  -p \
		  *.cc \
		  *.ttcn \
		  *.ttcnpp

sed -i 's#^CPPFLAGS =.*#CPPFLAGS += -D$(PLATFORM) -I. -I$(TTCN3_DIR)/include -I$(TTCN3_DIR)/include/titan#' Makefile

sed -i 's/^CXXFLAGS =.*/CXXFLAGS += -O3 -Wno-misleading-indentation -Wno-pointer-arith/' Makefile

sed -i 's#^LDFLAGS =.*#LDFLAGS += -L$(TTCN3_DIR)/lib/titan#' Makefile

sed -i 's/^COMPILER_FLAGS =/COMPILER_FLAGS += -D -g/' Makefile

sed -i 's/\($(CXX) -c\)/\t@echo \"   CXX\t\" $<\n\t@\1/' Makefile

sed -i 's/\(if $(CXX)\)/\@echo "   LD\t" $(EXECUTABLE)\n\t@\1/' Makefile

echo "libutils_ext.cc: libutils.hh" >> Makefile

echo >> Makefile

echo "port_cons.cc port_cons.hh: port_cons_types.hh" >> Makefile

make -j $(nproc)
