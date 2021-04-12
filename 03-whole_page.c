/**
 * Esercizio 3
 *
 * Definire staticamente un buffer in grado di contenere tutta la pagina
 * web mappata alla root di google e riportare tutta la pagina sul buffer
 * senza effettuare copie di bytes.
 * I dati devono essere scirri una sola volta dalla primitiva read() nel buffer.
 * Al termine una sola printf stampa tutto il buffer contenete tutta la pagina.
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

    char response[55000];
    
    //This variable stores the actual buffer (called response) usage
    unsigned int index = 0;
    
    unsigned int bytes_readed = 0; 
    
    do{
        //read() returns 0 when EOF is reached
        bytes_readed = read(s,&response[index],14800);
        index+= bytes_readed;

    }while(bytes_readed>0);

    if(bytes_readed==-1){
        perror("Read failed");
        return -1;
    }

    //String terminator
    response[index]='\0';

    //Printing out the result
    printf("%s\n", response);

    printf("\nBytes readed = %d\n",index);

}

