SRCS_DIR:=src
BUILD_DIR:=bin
CURRENT_DIR := $(shell pwd)
MKDIR=mkdir -p

CC=gcc
CFLAGS=-Wall -g -O2

CCSOURCES=$(wildcard $(SRCS_DIR)/*.c)

all: directories servers

servers: ${CCSOURCES}
	$(CC) $(CFLAGS) ${SRCS_DIR}/sp_st_blocking.c include/http-parser/http_parser.c -o ${BUILD_DIR}/sp_st_blocking.c

directories:
	${MKDIR} ${BUILD_DIR}

ctags:
	ctags -R --exclude=.git --exclude=Makefile --exclude=README.md .

clean:
	rm -rf bin tags