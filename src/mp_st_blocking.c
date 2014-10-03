#include "mp_st_blocking.h"


void make_path_from_url(const char* URL, char* file_path){

        strcpy(file_path, ROOT_FOLDER);
        strcat(file_path, URL);
        //if we need to fetch index.html
        if(strcmp(URL, "/") == 0){
            strcat(file_path, "index.html");
        }
}

/*
 * Read file content to a a char array
 * Return file size on success and -1 on failure
 */

ssize_t read_file_content(const char* file_path, char **buffer){

	FILE* fp;
	size_t file_size;

	fp= fopen(file_path, "rb");
	//Failed openning the file
	if(!fp)
		return -1;

	//Determine the file size
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	//Allocate memory for buffer
	*buffer = calloc( 1, file_size + 1);
	if(!*buffer){
		fclose(fp);
		LOG_ERROR("Error allocation buffer!");
		return -1;
	}

	//Copy file content into the buffer
	if(fread(*buffer, file_size, 1, fp) != 1){
		fclose(fp);
		free(*buffer);
		LOG_ERROR("File read failed");
		return -1;
	}

	fclose(fp);

	return file_size;
}
ssize_t read_http_request(int fd, void *vptr, size_t n){

	size_t nleft;
	ssize_t nread;
	char * ptr;

	ptr = vptr;
	nleft = n;

    while(nleft >0){
    	if( (nread = recv(fd, ptr, INPUT_BUFFER_LENGTH - 1, 0)) <0){
    		if (errno == EINTR)
    			nread =0;
    		else
    			return (-1);
    	}else if(nread ==0)
    		break;

    	nleft -= nread;

    	//If we are at the end of the http_request
    	if(ptr[nread-1] == '\n' && ptr[nread-2] == '\r' && ptr[nread-3] == '\n' && ptr[nread-4] == '\r')
    		break;

    	ptr += nread;

    }

    return (n-nleft);
}

ssize_t writen(int fd, const void *vptr, size_t n){
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;

	while(nleft > 0){
		if( (nwritten = send(fd, ptr, nleft, 0)) <= 0){
			if(errno == EINTR)
				nwritten = 0; /* If interrupted system call => call the write again */
			else
				return (-1);
		}
		nleft -= nwritten;
		ptr += nwritten;

	}

	return (n);
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
    ssize_t file_size ;
    char* buffer; //buffer used to read file


    if(parser->method == 1)
    {
        //Generate the URL which will be used to retrieve the file
        sprintf(URL, "%.*s", (int)size, header);

        //create the file path
        char file_path[255];
        make_path_from_url(URL, file_path);
       //open the file
        file_size = read_file_content(file_path, &buffer);

        //If we fail to open the file, simply return 404 !
        if(file_size == -1){

            writen(*sockfd, RESPONSE_NOT_FOUND, sizeof(RESPONSE_NOT_FOUND));
        }else{
        	//If file exists write the response type and file content to socket
        	writen(*sockfd, RESPONSE_OK, sizeof(RESPONSE_OK));
        	writen(*sockfd, buffer, file_size);
        	free(buffer); //buffer is being calloced inside read_file_content
        }

    }else{
        //Method is not allowed
        writen(*sockfd, RESPONSE_METHOD_NOT_ALLOWED, sizeof(RESPONSE_METHOD_NOT_ALLOWED));
    }

    return 0;
}

void intHandler(int sig){
	close(ssockfd);
	exit(1);
}
void chldHandler(int sig){
	while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

/* handle connection after accept */
void handle_connection(int csockfd){

    http_parser *parser = (http_parser *) malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);

    char buffer[INPUT_BUFFER_LENGTH]; //read buffer from the socket
    bzero(buffer, INPUT_BUFFER_LENGTH);
    size_t nparsed;
    ssize_t nrecvd; //return value for for the read() and write() calls.

    //Since we only accept GET, just try to read INPUT_BUFFER_LENGTH
    nrecvd = read_http_request(csockfd, buffer, INPUT_BUFFER_LENGTH -1);
    if(nrecvd<0)
    	LOG_ERROR("Error reading from socket");


    //pass the socket to http_parser
    parser->data = (void *) &csockfd;
    nparsed = http_parser_execute(parser, &settings, buffer, nrecvd);
    if(nparsed != nrecvd)
    	LOG_ERROR("Erorr in Parsing the request!");

    close(csockfd);
    free(parser);
}

int main() {
	pid_t pid;
    struct sockaddr_in serv_addr; //structure containing an internet address
    bzero((char*) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);


    //handle SIGINT to close the main socket
    signal(SIGINT, intHandler);
    //handle SIGCHLD
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
      perror(0);
      exit(1);
    }

    settings.on_url = on_url;

    ssockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ssockfd < 0) {
        LOG_ERROR("ERROR opening socket");
        exit(1);
    }


    if(bind(ssockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0) {
        LOG_ERROR("Error on binding");
        exit(1);
    };

    listen(ssockfd, 128);
    while(1) {
        int csockfd = accept(ssockfd, NULL, NULL);

        if (csockfd < 0) {
        	if(errno == EINTR)
        		continue;
        	else{
        		LOG_ERROR("Error on accept");
            	exit(1);
        	}
        }

        //if we are in child
        if( (pid = fork()) == 0){
        	close(ssockfd); //close the listening socket in child
        	handle_connection(csockfd);
        	exit(0);
       }else{
    	   //in parent close the connected socket
    	   //and continue listenting
    	   close(csockfd);
       }
    }
    close(ssockfd);
    return 0;
}
