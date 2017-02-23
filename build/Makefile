# Path to top of source tree
srcdir=..

# Installation directories
prefix=/usr/local
includedir=$(prefix)/include
dataroodir=$(prefix)/share
docdir=$(datarootdir)/doc
mandir=$(datarootdir)/man

.PHONY: clean all realclean test

public_includes:=optional.h uninitialized.h eitherm.h

all: unittest

CXXFLAGS+=-std=c++11 -g -pthread

# directories

srcdir=..
GTEST_DIR=$(srcdir)/gtest/googletest

vpath %.h $(srcdir)/optionalm
vpath test% $(srcdir)/test

# gtest includes

CPPFLAGS=-isystem $(GTEST_DIR)/include -I$(srcdir)

# build gtest

GTEST_HEADERS=$(GTEST_DIR)/include/gtest/*.h $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SOURCES=$(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o: CPPFLAGS+=-I$(GTEST_DIR)
gtest-all.o: $(GTEST_SOURCES)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o: CPPFLAGS+=-I$(GTEST_DIR)
gtest_main.o: $(GTEST_SOURCES)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(GTEST_DIR)/src/gtest_main.cc

libgtestmain.a: gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# build tests

unittest: CPPFLAGS+=-I$(srcdir)/include
unittest: LDLIBS+=-L. -lgtestmain
unittest: test.cc test_uninitialized.cc test_optional.cc test_common.h test_either.cc optional.h either.h uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(filter %.cc, $^) $(LDFLAGS) $(LDLIBS) 

# run tests

test: unittest
	for test in $^; do ./$$test; done

# install

install:
	$(INSTALL) -m 644 $(srcdir)/*.h $(DESTDIR)$(includedir)

# clean up

clean:
	rm -f gtest-all.o gtest_main.o

realclean: clean
	rm -f unittest libgtestmain.a


