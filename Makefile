SHELL := /bin/sh -xe
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

$(shell mkdir -p $(OUTDIR) $(EXTERNALDIR))

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TEST_OUT)

$(TEST_OUT): $(TEST_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

compile-db:
	bear -- make all

greatest:
	wget -NP $(EXTERNALDIR) https://raw.githubusercontent.com/silentbicycle/greatest/fbbf9818ec72578289716bf6002b11fd25185e02/greatest.h

deps: greatest

clean:
	rm -rf $(OUTDIR)/*

distclean:
	rm -rf $(OUTDIR)/*
	rm -rf $(EXTERNALDIR)/*

.PHONY: all clean deps greatest distclean compile-db test
