#! /usr/bin/make

CCFLAGS=-std=c++14 -O3 -D_GLIBCXX_DEBUG
SRCS=Cloudberry/CBLexer.cpp Cloudberry/CBParser.cpp Cloudberry/CBCompilerPython.cpp Cloudberry/Cloudberry.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY: all clean install

all: cbc

clean:
	rm $(OBJS)

install: cbc
	cp cbc /usr/bin/cbc

cbc: $(OBJS)
	$(CXX) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) -c $(CCFLAGS) -o $@ $<
