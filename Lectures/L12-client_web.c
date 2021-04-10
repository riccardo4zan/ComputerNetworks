/**
 * From socket documentation of Linux Programmer's Manual
 * You can visit it opening a terminal and writing:         man socket
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

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
    int tmp_error = 0;
    if(s==-1){
        tmp_error = errno;
        //System function Print ERRor
        perror("Socket failed");
        printf("errno=%d\n", tmp_error);
    }

}

