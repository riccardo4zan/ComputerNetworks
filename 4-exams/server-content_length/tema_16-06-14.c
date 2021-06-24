/**
 * Implementare la gestione dell'header content length
 * descritto in RFC 1945 sezione 10.4
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
    char * n;
    char * v;
}h[100];

int main()
{
    struct sockaddr_in addr,remote_addr;
    int i,j,k,s,t,s2,len;
    int c;
    FILE * fin;
    int yes=1;
    char * method, *path, *ver;
    char request[5000],response[10000];

    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){ perror("Socket fallita"); return 1; }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8777);
    addr.sin_addr.s_addr = 0;
    t = setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));

    if (t==-1){perror("setsockopt fallita"); return 1;}

    if ( bind(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {perror("bind fallita"); return 1;}

    if ( listen(s,5) == -1 ) { perror("Listen Fallita"); return 1; }

    len = sizeof(struct sockaddr_in);

    while(1){

        s2 =  accept(s, (struct sockaddr *)&remote_addr,&len);
        if ( s2 == -1 ) { perror("Accept Fallita"); return 1;}

        t = read(s2,request,4999);

        if ( t == -1 ) { perror("Read fallita"); return 1;}

        request[t]=0;

        printf("%s",request);
        //Parsing della command line HTTP
        method = request;
        for(i=0;request[i]!=' ';i++); request[i]=0; path = request+i+1;
        for(i++;request[i]!=' ';i++); request[i]=0; ver = request+i+1;
        for(i++;request[i]!='\r';i++); request[i]=0;
        printf("method=%s path=%s ver=%s\n",method,path,ver);

        if ((fin = fopen(path+1,"rt"))==NULL){
            sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
            write(s2,response,strlen(response));
        }
        else {
            
            //INIZIO delle modifiche
            int counter=0;              //Conta il numero di caratteri di cui sarà composta la response
            while ( (c = fgetc(fin)) != EOF ){
                counter++;
            }
            fclose(fin);                //Chiudo e ri apro il file
            fin = fopen(path+1, "rt");
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", counter-1);
            //FINE delle modifiche
            
            write(s2,response,strlen(response));
            
            while ( (c = fgetc(fin)) != EOF ){ 
                write(s2,&c,1);
                printf("Entro");
            }
            fclose(fin);	
        }
        close(s2);
    }
}

