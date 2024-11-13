.PHONY: all install uninstall

CC := gcc
CFLAGS := -std=c99 -pedantic -O0 -g3 -fsanitize=address -D_DEFAULT_SOURCE -Wall
INSTALL_DIR := /usr/bin

all: lithic

install: lithic
	cp lithic $(INSTALL_DIR)

uninstall:
	rm $(INSTALL_DIR)/lithic

lithic: lithic.c
	$(CC) $(CFLAGS) -o $@ $<
