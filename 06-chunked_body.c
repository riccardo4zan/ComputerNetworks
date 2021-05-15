/**
 * https://datatracker.ietf.org/doc/html/rfc2616#page-25
 * 
 * 
 * Linux's Programmer manual, references:
 * man socket
 * man 7 ip
 * man 2 connect 
 * man htons
 * man read
 * man 2 write
 * man printf
 * man bzero
 * man strtol
 */

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

// C structure to contain HTTP num_head
struct header{
    char * n;
    char * v;
} h[100];

int main() {
    /* 
       SOCKET INITIALIZATION : socket (domain, type, protocol)
           domain : AF_INET --> IPv4 Internet Protocols
           type : SOCK_STREAM --> connection-based byte stream
           protocol: 0 --> tcp stream socket
     */
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
        // error while creating the socket, print it and end the execution
        // using errno sys variable (<errno.h> necessary)
        int errorcode = errno;
        perror("Socket fallita");
        printf("errno=%d\n", errorcode);
        return 1;
    }

    /*
      CONNECTION OPENING : connect(sockfd, addr, addrlen)
          sockfd : socket file descriptor
          addr : sockaddr struct pointer containing the peer address
                 we declare a sockaddr_in struct and then cast to (struct sockaddr*)
          addrlen: length of the address struct
     */
    unsigned char targetip[4] = { 142,250,180,14 };
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int*) targetip;  
    if ( -1 == connect(s, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)))
	    perror("Connect fallita");

    /*
      SEND HTTP REQUEST : write(sockfd, buffer, count)
          sockfd : socket file descriptor
          buffer : pointer to the first byte to be sent
          count : number of bytes to be sent
     */
    // write the HTTP request in the request buffer
    char request[5000], response[10000];  
    sprintf(request, "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n");
    // send the request to the server
    if ( -1 == write(s,request,strlen(request))) {
        // could not send the request, print the error and terminate the program
        perror("write fallita"); 
        return 1;
    }
    // clean the header structures array
    bzero(h, sizeof(struct header)*100);
    // status line is the first part of the response
    // BE CAREFUL response is needed here
    char* statusline = h[0].n = response;
         
    /* 
     HEADER PARSING
         check the corresponding HTTP grammar to understand the underlying logic
     */
    int is_chunked = 0;
    int content_length = -1;
    int num_head = 0;
    for(int j=0; read(s, response+j, 1); j++) {
        if((response[j] == ':') && (h[num_head].v == 0)) {
            // reached the end of the header name
            // replace the ':' with a string terminator and initialize the header value pointer
            // +2 to trim the space after the ':'
            response[j] = 0;
            h[num_head].v = response + j + 2;
        }
        else if((response[j] == '\n') && (response[j-1] == '\r')) {
            // end of the header name
            response[j-1] = 0;
            // if there are no longer num_head, break
            if(h[num_head].n[0] == 0) break;

            if(!is_chunked && num_head != 0 && !strcmp(h[num_head].n, "Content-Length"))
                // if content-length header is found, get the content length
                content_length = atoi(h[num_head].v);
            
            if(content_length < 0 && num_head != 0 && !strcmp(h[num_head].n, "Transfer-Encoding") && !strcmp(h[num_head].v, "chunked"))  
                // if chunked header is found, set the corresponding flag
                is_chunked = 1;

            // set the pointer to the next header name
            h[++num_head].n = response + j + 1;
        }
    }

    /*
     DYNAMIC BODY ALLOCATION
     the buffer for the body is initially created with a standard size of 10kB
     then, if the body length is known, resize it
     chunked body parsing is also implemented in this section
    */
    char * body = (char*) malloc(10000);
    int bytes_read;
    if(!is_chunked) {
        // if the Content-length header is specified, realloc the body buffer to fit
        // the actual size of the body
        if(content_length > 0) body = (char*) realloc(body, content_length);
        // otherwise, update the content_length value to the size of the body buffer
        else content_length = 10000;
        // write the body to the buffer
        for(int i=0; bytes_read=read(s, body+i, content_length-i); i+=bytes_read);
    } else {
        
        /*
         CHUNKED BODY MANAGEMENT
         */
        content_length = 0;
        int size;
        // define a char array to store the size of chunks
        char chunk_size[50];
        // intialize first two characters of the size to "0x", in order to make the conversion easier
        chunk_size[0] = '0';
        chunk_size[1] = 'x';
        do {
            // parse the hexadecimal size of the chunk
            // stop the parsing when get an CRLF (case without chunk-extension) or when
            // an ; is found (case with chunk-extension)
            int ext = -1;
            for(int i=2; bytes_read = read(s, chunk_size+i, 1); i++) {
                // if an ';' is retrived, save the position (extension management)
                if(chunk_size[i] == ';') ext = i;
                // if the end of the chunk size is reached
                if(chunk_size[i] == '\n' && chunk_size[i-1] == '\r') {
                    // set terminator and exit from the cicle
                    if(ext < 0) chunk_size[i-1] = 0;
                    else chunk_size[ext] = 0;
                    break;
                }
            }
            // convert the hexadecimal string in decimal
            size = strtol(chunk_size, NULL, 16);
            // if it's not the last-chunk
            if(size > 0) {
                // realloc the body buffer to fit the new content length
                body = (char*) realloc(body, content_length + size + 2);
                // write the chunk data into the body buffer
                for(int j=0; (bytes_read=read(s, body + content_length + j, size + 2 - j))>0; j+=bytes_read);
                // sum the size of the chunk to the total body size
                content_length += size;
            }

         } while(size > 0); 
    }

    // print the body
    printf("%s", body);
}
