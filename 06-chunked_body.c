/**
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

    //Additions

    free(buffer);
    buffer=(char*)malloc(50);

    /**
     * Reading the first line of the chunked body, until CRLF is found
     * is possible that the chunked body contains chunked header but we are
     * not intrested in it. It's guaranteed by the grammar that the first number 
     * is the exadecimal rappresentation of how long the chunked body is.
    */

    unsigned int total_body_length=1;
    char* entity_body = (char*)malloc(1);

    unsigned int chunk_length;

    //This is a temporary buffer to store the exadecimal value of chunk length
    bzero(buffer,50);
    char* tmp = buffer;

    //Reading chunk length
    for(int i=0;read(s,buffer+i,1);i++){
        if((buffer[i]=='\n') && (buffer[i-1]=='\r')){
            buffer[i-1]=0;
            break;
        }
    }
    chunk_length = strtol(tmp,NULL,16);
    total_body_length += chunk_length;
    printf("Chunk length: %d\n",chunk_length);

    while(chunk_length>0){ 
        //Allocating the space and reading entity body
        entity_body = (char*)realloc(entity_body,total_body_length); 
        read(s,entity_body,chunk_length);
       
       	/*
        //Look for another chunk length
        bzero(buffer,50);
        int t;
        for(int i=0;t = read(s,buffer+i,1);i++){     
            printf("%d",t);
             if((buffer[i]=='\n') && (buffer[i-1]=='\r')){
                buffer[i-1]=0;
            }
        }
        if(t>0){
            chunk_length = strtol(tmp,NULL,16);
            total_body_length += chunk_length;
            printf("Chunk length: %d\n",chunk_length);
        } else*/ 
            break;
    }

    
    //Terminate the entity body and print it out
    entity_body[total_body_length]=0;
    printf("%s",entity_body);

}
