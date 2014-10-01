#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "../include/http-parser/http_parser.h"

#define PORT 8800
#define INPUT_BUFFER_LENGTH 80*1024

#define ROOT_FOLDER "/home/saman/ClionProjects/www"

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

void make_path_from_url(const char* URL, char* file_path){

        strcpy(file_path, ROOT_FOLDER);
        strcat(file_path, URL);
        //if we need to fetch index.html
        if(strcmp(URL, "/") == 0){
            strcat(file_path, "index.html");
        }
}

/*
* Ignore all headers except METHOD and Request URI
* We only handle GET requests, and parse the URI and return the proper file.
* If method is not GET, return 405 METHOD not allowed,
* Otherwise find the file and return the content (/ means /index.html).
* If there is no such file return 404 !
* */
int on_url(http_parser* parser, const char* header, long unsigned int size){
    //We only handle GET Requests
    char URL[size];
    int* sockfd = (int*)parser->data;
    int bytes_read;
    char buffer[256];


    if(parser->method == 1)
    {
        //Generate the URL which will be used to retrieve the file
        sprintf(URL, "%.*s", (int)size, header);

        //create the file path
        char file_path[255];
        make_path_from_url(URL, file_path);
       //open the file
        int fd;
        fd = open(file_path, O_RDONLY);

        //If we fail to open the file, simply return 404 !
        if(fd == -1){

            write(*sockfd, RESPONSE_NOT_FOUND, sizeof(RESPONSE_NOT_FOUND));
            close(fd);
            return 0;
        }

        //If file exists write the response type and file content to socket
        write(*sockfd, RESPONSE_OK, sizeof(RESPONSE_OK));
        while(1){
            bytes_read = read(fd, buffer, 255);
            if(bytes_read == 0)
                break;
            if(bytes_read < 0) {
                //errors ! handle them properly
            }
            void *p = buffer;
            while(bytes_read > 0){
               int bytes_written = write(*sockfd, p, bytes_read);
                if(bytes_written <=0){
                    //handle errors
                }
                bytes_read -= bytes_written;
                p += bytes_written;
            }
        }
        close(fd);

    }else{
        //Method is not allowed
        write(*sockfd, RESPONSE_METHOD_NOT_ALLOWED, sizeof(RESPONSE_METHOD_NOT_ALLOWED));
        return 0;
    }
    return 0;
}

int main() {
    int sockfd, newsockfd; //listens on sockfd and new connection on sockfd
    socklen_t clilen;
    char buffer[INPUT_BUFFER_LENGTH]; //read buffer from the socket
    struct sockaddr_in serv_addr, cli_addr; //structure containing an internet address
    ssize_t n; //return value for for the read() and write() calls.

    //http-parser settings (https://github.com/joyent/http-parser)
    http_parser_settings settings;
    settings.on_url = on_url;
    http_parser *parser = (http_parser *) malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_ERROR("ERROR opening socket");
        exit(1);
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0) {
        LOG_ERROR("Error on binding");
        exit(1);
    };

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while(1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        parser->data = (void *) &newsockfd;

        if (newsockfd < 0) {
            LOG_ERROR("Error on accept");
            exit(1);
        }

        bzero(buffer, INPUT_BUFFER_LENGTH);
        n = read(newsockfd, buffer, INPUT_BUFFER_LENGTH - 1);

        if (n < 0) LOG_ERROR("Error reading from socket");
        size_t nparsed;
        parser->data = (void *) &newsockfd;
        nparsed = http_parser_execute(parser, &settings, buffer, n);

        close(newsockfd);
    }
    close(sockfd);
    return 0;
}