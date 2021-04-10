/**
 * From socket documentation of Linux Programmer's Manual
 * You can visit it opening a terminal and writing:             man socket
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

int main(){

    //int socket(int domain, int type, int protocol);

    int s = socket(AF_INET, SOCK_STREAM, 0);

    /**
     * The minimum number is 3:
     * 0 is stdin
     * 1 is stdout
     * 2 is stderr
     */ 
    printf("Socket number: %d\n",s);

    /**
     * According to the documentation, in case of error
     * the value returned by the socket function is -1.
     * 
     * If an error occurs, this code fragment prints out
     * information about the error.
    */
    int socket_status = 0;
    if(s==-1){
        socket_status = errno;
        //System function Print ERRor
        perror("Socket failed");
        printf("errno=%d\n", socket_status);
    }
    
    /**
     * Constructing the IP Packet 
     * For reference visit:                                     man 7 ip
     * For reference visit:                                     man 2 connect
     */
    
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    //216.58.213.100 google.com
    unsigned char target_ip[4] = {216, 58, 213, 100};
    //Bytes are inserted in BIG endian (network order)
    addr.sin_addr.s_addr = *(unsigned int*)target_ip;

    int connect_status = connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if(connect_status==-1) printf("Error while connecting");
}

