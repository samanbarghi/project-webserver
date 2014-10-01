/*
 * sp_st_blocking.h
 *
 *  Created on: Oct 1, 2014
 *      Author: Saman Barghi
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>


#include "../include/http_parser.h"

#ifndef SP_ST_BLOCKING_H_
#define SP_ST_BLOCKING_H_



#define PORT 8800
#define INPUT_BUFFER_LENGTH 4*1024 //4 KB

#define ROOT_FOLDER "/home/saman/tmp/www"

/* HTTP responses*/
#define RESPONSE_METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed\r\n"
#define RESPONSE_NOT_FOUND "HTTP/1.0 404 Not Found\n" \
                            "Content-type: text/html\n" \
                            "\n" \
                            "<html>\n" \
                            " <body>\n" \
                            "  <h1>Not Found</h1>\n" \
                            "  <p>The requested URL was not found on this server.</p>\n" \
                            " </body>\n" \
                            "</html>\n"

// To avoid complication only return a html files (a single index.html file will be servred)
#define RESPONSE_OK "HTTP/1.0 200 OK\r\n" \
                    "Content-type: text/html\r\n" \
                    "\r\n"

/* Logging */
#define LOG(msg) puts(msg);
#define LOGF(fmt, params...) printf(fmt "\n", params);
#define LOG_ERROR(msg) perror(msg);



int ssockfd; //listens on ssockfd and new connection on csockfd

//http-parser settings (https://github.com/joyent/http-parser)
http_parser_settings settings;

#endif /* SP_ST_BLOCKING_H_ */
