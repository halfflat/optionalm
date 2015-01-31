.PHONY: clean all realclean test

all: test_optionalm

clean:
	rm -f gtest-all.o gtest_main.o

realclean: clean
	rm -f test_optionalm libgtestmain.a

# compiler

CXX=clang++
CXXFLAGS=-stdlib=libc++ -std=c++11 -g -pthread

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
test_optionalm: test_optionalm.cc optionalm.h libgtestmain.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) 

# run tests

test: test_optionalm
	./$<


