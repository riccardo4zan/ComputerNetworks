/**
 * Linux's Programmer manual, references:
 * man socket
 * man 7 ip
 * man 2 connect 
 * man htons
 * man read
 * man 2 write
 * man printf
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

/**
 * Struct used to read HTTP headers
 * from the HTTP response
 */
struct header{
    char *name;
    char *value;
};

int main(){

    int s = socket(AF_INET, SOCK_STREAM, 0);

    int socket_status = 0;
    if(s==-1){
        socket_status = errno;
        perror("Socket failed");
        printf("errno=%d\n", socket_status);
        return 1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    //216.58.213.100 google.com
    unsigned char target_ip[4] = {216, 58, 213, 100};
    addr.sin_addr.s_addr = *(unsigned int*)target_ip;

    int connect_status = connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(connect_status==-1){
        perror("Connection failed");
        return 1;
    }
    
    char request[5000];

    sprintf(request, "GET /pulto HTTP/1.0\r\n\r\n");

    int request_status = write(s,request,strlen(request));
         
    if(request_status == -1){
        perror("Request failed");
        return 1;
    }

    char response[1000000];

    //First line of an HTTP response
    char *statusline;

    struct header response_headers[100];
    
    unsigned int j=0, headers_length=0;

    //The first line of HTTP header is 'an exception'
    statusline = response_headers[0].name = response;

    //Read return 0 when EOF reached, reading 1 byte at time
    while( read(s,response+j,1) ){

        //Inserting a new value in the struct
        if(response[j] == ':' && (response_headers[headers_length].value==0) ){
            response[j]='\0';
            response_headers[headers_length].value=response+j+1;
        }

        //Inserting a new name in the struct
        if(response[j]=='\n' && response[j-1]=='\r'){
            response[j-1]='\0';

            //Headers terminated when two CRLF are found subsequently
            if(response_headers[headers_length].name[0]=='\0') break;

            //Point to the next one after CRLF
            headers_length++;
            response_headers[headers_length].name=response+j+1;
        }

        j++;

    }

    printf("Status line: %s\n", statusline);
    for(int i=1;i<headers_length;i++){
        printf("%s --> %s\n",response_headers[i].name, response_headers[i].value);
    }
    
}

