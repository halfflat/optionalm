.PHONY: clean all realclean test

tests:=test_optionalm test_uninitialized test_eitherm

all: $(tests)

clean:
	rm -f gtest-all.o gtest_main.o

realclean: clean
	rm -f $(tests) libgtestmain.a

# compiler

CXX=clang++
#CXX=g++

CXXFLAGS=-std=c++11 -g -pthread
ifeq ($(CXX),clang++)
CXXFLAGS+=-stdlib=libc++
endif

# directories

top=..
GTEST_DIR=$(top)/gtest-1.7.0

vpath %.h $(top)/include
vpath test% $(top)/test

# gtest includes

CPPFLAGS=-isystem $(GTEST_DIR)/include

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

test_optionalm: CPPFLAGS+=-I$(top)/include
test_optionalm: LDLIBS+=-L. -lgtestmain
test_optionalm: test_optionalm.cc optionalm.h uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

test_eitherm: CPPFLAGS+=-I$(top)/include
test_eitherm: LDLIBS+=-L. -lgtestmain
test_eitherm: test_eitherm.cc eitherm.h uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

test_uninitialized: CPPFLAGS+=-I$(top)/include
test_uninitialized: LDLIBS+=-L. -lgtestmain
test_uninitialized: test_uninitialized.cc uninitialized.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

# run tests

test: test_eitherm test_optionalm test_uninitialized
	for test in $^; do ./$$test; done


