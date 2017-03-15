CXX = clang++
CXXFLAGS = --std=c++0x -g
LDLIBS = -lstdc++

all: example

example.o: example.cpp http-cppx0.hpp socket-wrapper.hpp
example: example.o
