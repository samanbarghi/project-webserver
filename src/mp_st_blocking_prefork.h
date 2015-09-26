/*
 * mp_st_blocking_prefork.h
 *
 *  Created on: Oct 3, 2014
 *      Author: saman
 */
#include "project_webserver_global.h"
#include "../include/ancillary.h"

#ifndef MP_ST_BLOCKING_PREFORK_H_
#define MP_ST_BLOCKING_PREFORK_H_


#define INITIAL_NUMBER_OF_PROCESSES 10

enum status {

};
typedef struct{
	pid_t pid; //forked process pid
	int sockfd[2]; //Socket pair returned by socketpair


} forked_process;

int ssockfd; //listens on ssockfd and new connection on csockfd

//http-parser settings (https://github.com/joyent/http-parser)
http_parser_settings settings;

#endif /* MP_ST_BLOCKING_PREFORK_H_ */
