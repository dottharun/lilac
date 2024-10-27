SHELL := /bin/sh -e
MAKEFLAGS += -B

CC = clang
CFLAGS = -std=c17 -Wall -Wextra -I$(EXTERNALDIR) -g
LDFLAGS =

SRC = src/main.c
OUTDIR = out
OUT = $(OUTDIR)/lilac
EXTERNALDIR = external

TEST_SRC = tests/test_runner.c
TEST_OUT = $(OUTDIR)/test_runner

WASM_OUT = out/lilac.js
WASM_FLAGS = -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["cwrap"]' -s INVOKE_RUN=0

$(shell mkdir -p $(OUTDIR) $(EXTERNALDIR))

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TEST_OUT)

$(TEST_OUT): $(TEST_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

wasm: $(WASM_OUT)

$(WASM_OUT): $(SRC)
	emcc $(CFLAGS) $(WASM_FLAGS) $< -o $@ $(LDFLAGS)

compile-db:
	bear -- make all

stb:
	wget -NP $(EXTERNALDIR) https://raw.githubusercontent.com/nothings/stb/1ee679ca2ef753a528db5ba6801e1067b40481b8/stb_ds.h

greatest:
	wget -NP $(EXTERNALDIR) https://raw.githubusercontent.com/silentbicycle/greatest/fbbf9818ec72578289716bf6002b11fd25185e02/greatest.h

gb:
	wget -NP $(EXTERNALDIR) https://raw.githubusercontent.com/gingerBill/gb/fd88428545cac94db72e93a1ff36c27153628874/gb_string.h

deps: greatest stb gb

clean:
	rm -rf $(OUTDIR)/*

distclean:
	rm -rf $(OUTDIR)/*
	rm -rf $(EXTERNALDIR)/*

.PHONY: all clean deps greatest distclean compile-db test
