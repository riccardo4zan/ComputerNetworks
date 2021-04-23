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

    //Reading the response
    char isHeaderCompleted = 0;
    char response[1000000];

    char *statusline;
    struct header response_headers[100];
    unsigned int index=0, headers_length=0;
    /**
     * The first line of the HTTP request is different 
     * from the others. Other lines follows the grammar 
     * name: value
     */
    statusline = response_headers[0].name = response;

    do{
        read(s,response+index,1);
        
        //The second condition is required to read the date field correctly
        if(response[index] == ':' && (response_headers[headers_length].value==0) ){
            response[index]='\0';
            response_headers[headers_length].value=response+index+1;
        }

        if(response[index]=='\n' && response[index-1]=='\r'){
            response[index-1]='\0';

            if(response_headers[headers_length].name[0]=='\0'){
                isHeaderCompleted=1;
                break;
            }

            headers_length++;
            response_headers[headers_length].name=response+index+1;
        }

        index++;

    }while(!isHeaderCompleted);

    //Reading from the header the content-length (if present)
    unsigned int body_length;
    for(int i=0;i<headers_length;i++){
        if(strcmp("Content-Length",response_headers[i].name)==0) body_length = atol(response_headers[i].value);
    }

    //Reading the body of the response
    char body[body_length];
    read(s,&body,body_length);

    //Printing headers
    printf("Status line: %s\n", statusline);
    for(int i=1;i<headers_length;i++){
        printf("%s --> %s\n",response_headers[i].name, response_headers[i].value);
    }

    //Printing body
    printf("%s", body);
    
}


