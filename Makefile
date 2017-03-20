CXX = clang++
CXXFLAGS = --std=c++0x -g
LDLIBS = -lstdc++

FILE_TO_H = xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@

all: example

page-main-js.h: page-main.js 
	$(FILE_TO_H)
	
page-index.h: page-index.html
	$(FILE_TO_H)
	
example.o: example.cpp http-cppx0.hpp socket-wrapper.hpp page-index.h page-main-js.h
example: example.o
