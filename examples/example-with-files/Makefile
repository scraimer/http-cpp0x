include Makefile.defs

GENERATED_HEADERS = pages/index.html.h pages/main.js.h pages/style.css.h
TARGETS = example.o example $(GENERATED_HEADERS)

all: example

FILE_TO_H = xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@

pages/%.h: pages/%
	$(FILE_TO_H)
	
example.o: example.cpp $(INCLUDE_DIR)/http-cppx0.hpp $(INCLUDE_DIR)/socket-wrapper.hpp $(GENERATED_HEADERS)
example: example.o

.PHONY: clean
clean:
	-rm -f $(TARGETS)
	
