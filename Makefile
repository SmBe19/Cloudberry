#! /usr/bin/make

CCFLAGS=-std=c++14 -O3
SRCS=Cloudberry/CBLexer.cpp Cloudberry/CBParser.cpp Cloudberry/Cloudberry.cpp
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
