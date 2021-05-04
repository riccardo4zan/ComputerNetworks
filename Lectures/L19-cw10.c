 /**
  * 
  * Linux's Programmer manual, references:
  * man socket
  * man 7 ip
  * man 2 connect 
  * man htons
  * man read
  * man 2 write
  * man printf
  */

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

struct header{
    char *n;
    char *v;
};

int main(){

    struct sockaddr_in addr;
    char request[5000], response[10000];
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

    sprintf(request, "GET /pluto HTTP/1.0\r\nConnection:keep-alive\r\n\r\n");

    if (-1 == write(s, request, strlen(request))){ perror("write fallita"); return 1;}

    //This array of struct stores the headers (100 at most)
    struct header h[100];

    char *statusline = h[0].n = response;

    //This variable stores the number of headers 
    int k=0;

    for (int j = 0; read(s, response + j, 1); j++){
        if (response[j] == ':' && (h[k].v == 0)){
            response[j] = 0;
            h[k].v = response + j + 1;
        }
        else if ((response[j] == '\n') && (response[j - 1] == '\r')){
            response[j - 1] = 0;
            if (h[k].n[0] == 0)
                break;
            h[++k].n = response + j + 1;
        }

    }

    printf("Status line = %s\n", statusline);

    //This variable stores the length of the response's body
    int entity_length = -1;

    //Printing out header
    for (int i = 1; i < k; i++){
        //Search if present Content-Length header
        if (strcmp(h[i].n, "Content-Length") == 0){
            entity_length = atoi(h[i].v);
            //printf("** Found entity length: (%d) ** ", entity_length);
        }
        //Printing header
        printf("%s ----> %s\n", h[i].n, h[i].v);
    }
    if (entity_length == -1) entity_length = 1000000;

    //This pointer points to the entity body
    char *entity = (char *)malloc(entity_length);

    //This variable stores the number of bytes readed at each call of read()
    unsigned int t=0;
    //This variable stores the effective entity length readed from the socket
    unsigned int j;
    for (j = 0; (t = read(s, entity + j, entity_length - j)) > 0; j += t)
        ;
    
    if (t == -1) { perror("Read fallita"); return 1;}

    printf("\nLunghezza dell'entity body letta dal socket = %d\n\n", j);

    //Printing out response's body
    for (int i = 0; i < j; i++)
        printf("%c", entity[i]);

}
