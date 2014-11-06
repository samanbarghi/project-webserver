SRCS_DIR:=src
BUILD_DIR:=bin
INCLUDE_DIR:=include
CURRENT_DIR := $(shell pwd)
MKDIR=mkdir -p

CC=gcc
CFLAGS=-Wall -g -O2

CCSOURCES=$(wildcard $(SRCS_DIR)/*.c)

all: directories servers

servers: ${CCSOURCES}
	$(CC) $(CFLAGS) ${SRCS_DIR}/sp_st_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_st_blocking
	$(CC) $(CFLAGS) ${SRCS_DIR}/mp_st_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/mp_st_blocking
	$(CC) $(CFLAGS) ${SRCS_DIR}/sp_mt_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_mt_blocking -lpthread

directories:
	${MKDIR} ${BUILD_DIR}

ctags:
	ctags -R --exclude=.git --exclude=Makefile --exclude=README.md .

clean:
	rm -rf bin tags
