# Path to top of source tree
srcdir=..

# Installation directories
prefix=/usr/local
includedir=$(prefix)/include
dataroodir=$(prefix)/share
docdir=$(datarootdir)/doc
mandir=$(datarootdir)/man

.PHONY: clean all realclean test

tests:=test_optionalm test_uninitialized test_eitherm
public_includes:=optionalm.h uninitialized.h eitherm.h

all: $(tests)

CXXFLAGS+=-std=c++11 -g -pthread
ifeq ($(CXX),clang++)
CXXFLAGS+=-stdlib=libc++
endif

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

test_optionalm: CPPFLAGS+=-I$(srcdir)/include
test_optionalm: LDLIBS+=-L. -lgtestmain
test_optionalm: test_optionalm.cc optionalm.h uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

test_eitherm: CPPFLAGS+=-I$(srcdir)/include
test_eitherm: LDLIBS+=-L. -lgtestmain
test_eitherm: test_eitherm.cc eitherm.h uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

test_uninitialized: CPPFLAGS+=-I$(srcdir)/include
test_uninitialized: LDLIBS+=-L. -lgtestmain
test_uninitialized: test_uninitialized.cc uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

# run tests

test: test_eitherm test_optionalm test_uninitialized
	for test in $^; do ./$$test; done

# install

install:
	$(INSTALL) -m 644 $(srcdir)/*.h $(DESTDIR)$(includedir)

# clean up

clean:
	rm -f gtest-all.o gtest_main.o

realclean: clean
	rm -f $(tests) libgtestmain.a


