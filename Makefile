SHELL := /bin/sh -xe
MAKEFLAGS += -B

CC = clang
CFLAGS = -std=c17 -Wall -Wextra -g
LDFLAGS =

SRC = src/main.c
OUTDIR = out
OUT = $(OUTDIR)/lilac

$(shell mkdir -p $(OUTDIR))

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

compile-db:
	bear -- make all

clean:
	rm -rf $(OUTDIR)/*

.PHONY: all clean deps stb distclean compile-db
