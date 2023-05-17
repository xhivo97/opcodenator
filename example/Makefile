CC := gcc
LDFLAGS := -lm

.PHONY: all clean

all: test_decoder

test_decoder: test_decoder.c decoder.h
	@ echo "building test"
	@ $(CC) $< -o $@

decoder.h: generate_decoder
	@ echo "generating decoder.h"
	@ ./$< > $@

generate_decoder: generate_decoder.c
	@ echo "building generator"
	@ $(CC) $(LDFLAGS) $< -o $@

clean:
	rm test_decoder decoder.h generate_decoder
