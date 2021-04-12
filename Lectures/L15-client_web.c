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

int main(){

    int s = socket(AF_INET, SOCK_STREAM, 0);
 
    //printf("Socket number: %d\n",s);

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
    //Bytes are inserted in BIG endian (network order)
    addr.sin_addr.s_addr = *(unsigned int*)target_ip;

    int connect_status = connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(connect_status==-1){
        perror("Connection failed");
        return 1;
    }

    //Client Request, asking for root file (default: index.html)
    
    char request[5000];

    sprintf(request, "GET / \r\n");

    int request_status = write(s,request,strlen(request));
         
    if(request_status == -1){
        perror("Request failed");
        return 1;
    }

    char response[1000000];

    unsigned int bytes_readed = 0; 
    
    do{
        //read() returns 0 when EOF is reached
        bytes_readed = read(s,response,999999);

        //printf("Bytes readed from response = %d\n", bytes_readed); 

        //Printing out server response

        //Inserting the string terminator at the END of the buffer (5000)
        response[bytes_readed]='\0';

        printf("%s\n", response);

    }while(bytes_readed>0);

    if(bytes_readed==-1){
        perror("Read failed");
        return -1;
    }
    

}

