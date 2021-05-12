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

#define MAX_LENGTH 10000

struct header{
    char *n;
    char *v;
};

int main(){

    struct sockaddr_in addr;
    char request[5000]; 
    unsigned char targetip[4] = {216, 58, 213, 100};

    //This variable contains the number of the socket opened by the SO
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1){
        int tmp = errno;
        perror("Socket fallita");
        printf("errno=%d\n", tmp);
        return 1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int *)targetip; // <indirizzo ip del server 216.58.213.100 >

    if (-1 == connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) perror("Connect fallita");
   
    printf("Socket number: %d\n", s);

	sprintf(request,"GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n");

    if (-1 == write(s, request, strlen(request))){ perror("write fallita"); return 1;}

    //This array of struct stores the headers (100 at most)
    struct header h[100];

    //Buffer is used to parse the response    
    char* buffer;
    buffer = (char*)malloc(MAX_LENGTH);

    char *statusline = h[0].n = buffer;

    //This variable stores the number of headers 
    int k=0;

    for (int i = 0; read(s, buffer + i, 1); i++){
        if (buffer[i] == ':' && (h[k].v == 0)){
            buffer[i] = 0;
            //Trim the first space
            h[k].v = buffer + i + 2;
        }
        else if ((buffer[i] == '\n') && (buffer[i - 1] == '\r')){
            buffer[i - 1] = 0;
            if (h[k].n[0] == 0)
                break;
            h[++k].n = buffer + i + 1;
        }

    }

    printf("Status line = %s\n", statusline);

    //Printing out header
    for (int i = 1; i < k; i++){
        //Search if present Content-Length header
        if (strcmp(h[i].n, "Transfer-Encoding") == 0 && strcmp(h[i].v,"chunked") == 0){
            printf("\n *** FOUND CHUNKED BODY ***\n");
        }
        //Printing header
        printf("%s ----> %s\n", h[i].n, h[i].v);
    }

    //Parsing the chunked body

    free(buffer);

    /**
     * Reading the first line of the chunked body, until CRLF is found
     * is possible that the chunked body contains chunked header but we are
     * not intrested in it. It's guaranteed by the grammar that the first number 
     * is the exadecimal rappresentation of how long the chunked body is.
    */

    unsigned int total_body_length=1;
    char* entity_body = (char*)malloc(1);

    //This is a temporary buffer to store the exadecimal value of chunk length
    char* tmp = (char*)malloc(50);;
    //This variable stores the decimal value of the chunk
    unsigned int chunk_length;

    do{
        //Reading chunk length
        for(int i=0;read(s,tmp+i,1);i++){
            if((tmp[i]=='\n') && (tmp[i-1]=='\r')){
                tmp[i-1]=0;
                break;
            }
        }
        chunk_length = strtol(tmp,NULL,16);

        //Allocating the space for the entity body
        unsigned int size = total_body_length+chunk_length+1;
        entity_body=(char*) realloc(entity_body,size);

        /**
         * Read byte for byte, reading the size of the chunk is not possible  
         */
        for(int i=0;i<chunk_length;i++){
            total_body_length++;
            read(s,entity_body+total_body_length,1);
        }
        
        /**
         * Is mandatory to discard 2 bytes to pass to the read function a pointer,
         * if NULL passed, it returns 0.
         */
        char discard[2];
        read(s,discard,2);
        //if(discard[0]=='\r' && discard[1]=='\n') printf ("FINE CHUNK\n");

    }while(chunk_length>0);

    /**
     * Printing out results
     */
    printf("Total length: %d\n",total_body_length);
    entity_body[total_body_length+1]=0;
    printf("%s",entity_body);

}
