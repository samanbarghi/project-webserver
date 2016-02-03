SRCS_DIR:=src
BUILD_DIR:=bin
INCLUDE_DIR:=include
CURRENT_DIR := $(shell pwd)
MKDIR=mkdir -p

CC=gcc
CXX=g++ --std=c++1y
CFLAGS=-Wall -g -ggdb -O2 -fno-omit-frame-pointer -I/usr/local/include -I/usr/local/include/uThreads

CCSOURCES=$(wildcard $(SRCS_DIR)/*.c)

all: directories webserver

servers: ${CCSOURCES}
	$(CC) $(CFLAGS) ${SRCS_DIR}/sp_st_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_st_blocking
	$(CC) $(CFLAGS) ${SRCS_DIR}/mp_st_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/mp_st_blocking
	$(CC) $(CFLAGS) ${SRCS_DIR}/sp_mt_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_mt_blocking -lpthread
#	$(CXX) $(CFLAGS) -L/home/saman/Research/research-projects/uThread/lib ${SRCS_DIR}/sp_mt_ut_blocking.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_mt_ut_blocking -luThread
	$(CXX) $(CFLAGS) ${SRCS_DIR}/sp_mt_ut_blocking_nomigrate.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/sp_mt_ut_blocking_nomigrate -luThreads

io:
	$(CXX) $(CFLAGS) ${SRCS_DIR}/io_test.c ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/io_test -luThreads

webserver:
	$(CXX) $(CFLAGS) ${SRCS_DIR}/webserver.cpp ${INCLUDE_DIR}/http_parser.c -o ${BUILD_DIR}/webserver -luThreads

directories:
	${MKDIR} ${BUILD_DIR}

ctags:
	ctags -R --exclude=.git --exclude=Makefile --exclude=README.md .

clean:
	rm -rf bin tags
