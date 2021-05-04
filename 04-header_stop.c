/**
 * Esercizio:  Effettuare la lettura dell'entity body acquisendo il 
 * valore dell' header content-length e fermando la lettura dell'entitiy body 
 * al raggiungimento del corrispondente numero di caratteri.  
 * 
 * Refence: https://tools.ietf.org/html/rfc1945
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
 * man atol
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 1000000  

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
    if(s==-1){
        int socket_status = errno;
        perror("Socket failed");
        printf("errno=%d\n", socket_status);
        return 1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    unsigned char target_ip[4] = {216, 58, 213, 100};      //216.58.213.100 google.com
    addr.sin_addr.s_addr = *(unsigned int*)target_ip;

    int connect_status = connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(connect_status==-1){
        perror("Connection failed");
        return 1;
    }
    
    char request[5000];
    sprintf(request, "GET /pluto HTTP/1.0\r\n\r\n");
    int request_status = write(s,request,strlen(request)); 
    if(request_status == -1){
        perror("Request failed");
        return 1;
    }

    //Reading the response headers
    char response[1000000];

    struct header response_headers[100];
    unsigned int headers_length=0;
    /**
     * The first line of the HTTP request is different 
     * from the others. Other lines follows the grammar 
     * name: value
     */
    char *statusline = response_headers[0].name = response;

    char isHeaderCompleted = 0; //used as bool, 0==false otherwise true

    for(int i=0;!isHeaderCompleted;i++){
        //Reading the HTTP response byte per byte
        read(s,response+i,1);
        
        //The second condition is required to read the date field correctly
        if(response[i] == ':' && (response_headers[headers_length].value==0) ){
            response[i]='\0';
            response_headers[headers_length].value=response+i+1;
        }

        else if(response[i]=='\n' && response[i-1]=='\r'){
            response[i-1]='\0';

            if(response_headers[headers_length].name[0]=='\0'){
                isHeaderCompleted=1;
                break;
            }

            headers_length++;
            response_headers[headers_length].name=response+i+1;
        }
    }

    //Reading from the header the content-length (if present)
    unsigned int body_length=-1;
    for(int i=0;i<headers_length;i++){
        if(strcmp("Content-Length",response_headers[i].name)==0){
            body_length = atol(response_headers[i].value);
        }
    }
    //If the size for the response's body is not declared, set it to max
    if(body_length==-1) 
        body_length=MAX_LENGTH;

    printf("The entity body length is  %d bytes \n",body_length);

    //Creating a pointer that references where the body starts in the buffer response
    char* body = (char*) malloc(body_length);

    unsigned int bytes_readed=0, offset=0;

    do{
        /**
         * read() returns 0 when EOF is reached or when body_length reaches 0,
         * it means that there is no more space in the buffer
         */ 
        bytes_readed = read(s,body+offset,body_length);
        offset+= bytes_readed;
        body_length-=bytes_readed;
    }while(bytes_readed>0);

    if(bytes_readed==-1){
        perror("Read failed");
        return -1;
    }

    //String terminator
    body[offset]='\0';

    //Printing headers
    printf("Status line: %s\n", statusline);
    for(int i=1;i<headers_length;i++){
        printf("%s --> %s\n",response_headers[i].name, response_headers[i].value);
    }

    //Printing body
    printf("%s\n", body);
    
}