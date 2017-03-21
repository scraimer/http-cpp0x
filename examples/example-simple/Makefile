include Makefile.defs

TARGETS = example.o example

all: example

FILE_TO_H = xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@

example.o: example.cpp $(INCLUDE_DIR)/http-cppx0.hpp $(INCLUDE_DIR)/socket-wrapper.hpp
example: example.o

.PHONY: clean
clean:
	-rm -f $(TARGETS)
	
