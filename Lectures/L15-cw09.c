/**
 * Client web for HTTP 0.9
 * Linux's Programmer manual, references:
 * man socket
 * man 7 ip
 * man 2 connect 
 * man htons
 * man read
 * man 2 write
 * man printf
 */

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>


int main(){

    struct sockaddr_in addr;

    char request[5000], response[1000000];

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

    if (-1 == connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) 
        perror("Connect fallita");

    printf("Socket number: %d\n", s);

    sprintf(request, "GET / \r\n");

    if (-1 == write(s, request, strlen(request))){
        perror("write fallita");
        return 1;
    }

    //This variable stores the number of bytes readed at each call of read()
    unsigned int t=0;
    while ((t = read(s, response, 999999)) > 0){
        //Inserting the string terminator after the read
        response[t] = 0;
        printf("%s",response);
        printf("\nBytes readed = %d\n", t);
    }

    if (t == -1){
        perror("Read fallita");
        return 1;
    }
}
