/**
 * Il codice di partenza che è stato fornito si trova in RDC1-21/1-lectures/5-sw21.c
 * QUESTO PROGRAMMA NON LEGGE CORRETTAMENTE QUANTO PRESENTE NEL BODY DELLA REQUEST COME PARAMETRO POST
 * PROBABILMENTE PER VIA DI UN ERRORE A RIGA 115 
 * Bisognerebbe provare a sostituire     while(t=read(s2,&request[j],to_read))
 * con                                   while(t=read(s2,request+j,to_read))
 */

// Svolgimento del tema d'esame (valutazione 29)

/**
 * Devo controllare che la richiesta sia di tipo POST, il path della richiesta corrisponda a /cgi-bin/command
 * successivamente devo effettuare il parsing dei parametri passati attraverso il metodo post (sono nell'entity body della request)
 * leggendo la lunghezza dell'entity body dall'header della request.
 * Devo poi creare la stringa di comando ed effettuare la chiamata a system.
 * Successivamente devo mandare in output il comando.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

int tmp;

struct header{
    char * n;
    char * v;
}h[100];

int main()
{
    struct sockaddr_in addr,remote_addr;
    int i,j,k,s,t,s2,len;
    char command[100];
    int c;
    FILE * fin;
    int yes=1;
    char * commandline;
    char * method, *path, *ver;
    char request[5000],response[10000];
    s =  socket(AF_INET, SOCK_STREAM, 0);

    if ( s == -1 ){ perror("Socket fallita"); return 1; }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12775);
    addr.sin_addr.s_addr = 0;
    t= setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));

    if (t==-1){perror("setsockopt fallita"); return 1;}
    if ( bind(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {perror("bind fallita"); return 1;}
    if ( listen(s,225) == -1 ) { perror("Listen Fallita"); return 1; }
    len = sizeof(struct sockaddr_in);

    while(1){
        s2 =  accept(s, (struct sockaddr *)&remote_addr,&len);
        if ( s2 == -1 ) { perror("Accept Fallita"); return 1;}
        bzero(h,100*sizeof(struct header *));
        commandline = h[0].n=request;
        for( j=0,k=0; read(s2,request+j,1);j++){
            if(request[j]==':' && (h[k].v==0) ){
                request[j]=0;
                h[k].v=request+j+1;
            }
            else if((request[j]=='\n') && (request[j-1]=='\r') ){
                request[j-1]=0;
                if(h[k].n[0]==0) break;
                h[++k].n=request+j+1;
            }
        }

        printf("Command line = %s\n",commandline);
        for(i=1;i<k;i++){
            printf("%s ----> %s\n",h[i].n, h[i].v);
        }

        method = commandline;
        for(i=0;commandline[i]!=' ';i++){} commandline[i]=0; path = commandline+i+1;
        for(i++;commandline[i]!=' ';i++); commandline[i]=0; ver = commandline+i+1;
        /* il terminatore NULL dopo il token versione è già stato messo dal parser delle righe/headers*/

        printf("method=%s path=%s ver=%s\n",method,path,ver);

        //Inizio delle modifiche

        if( strcmp(method,"POST")==0 && strncmp(path,"/cgi-bin/",9) == 0 ){
            //Parsing dell'entity body della request

            //Controllo se è presente l'header Content-Type
            for(i=0;i<k;i++){
                if( strcmp(h[i].n, "Content-Type")==0 && strcmp(h[i].v," application/x-www-form-urlencoded")==0 ){

                    char* request_body = response+j;
                    printf("\n** Post request found**\n");

                    //Effettuo il parsing di quanto inserito all'interno del body
                    int rbl,ai;
                    for(ai=0;ai<k;ai++){
                        if( strcmp(h[ai].n,"Content-Length")==0 ){
                            rbl = atoi(h[ai].v);
                            break;
                        }
                    }
                    printf("** Request body length: %d\n", rbl);

                    //leggo il body della request (NON FUNZIONA), legge correttamente la
                    //dimensione dei parametri passati attraverso
                    int to_read=rbl;
                    printf("** value of j=%d, length to read=%d\n",j,to_read);
                    while(t=read(s2,&request[j],to_read)){
                        j+=t;
                        to_read-=t;
                    }
                    printf("** value of j=%d, length to read=%d\n",j,to_read);
                    printf("** DEBUG %s\n",request_body);

                    //Metto nel codice il comando che dovrebbe esser stato letto dal form:
                    //request_body dovrebbe contenere comando=ls&param1=-l&param2=-a
                    sprintf(request_body,"comando=ls&param1=-l&param2=-a");
                    printf("** %s\n",request_body);

                    //effettuo il parsing del request_body
                    int len = strlen(request_body);
                    char *cmd, *arg1, *arg2;

                    for(i=0 ; i<len && request_body[i]!='=';i++);
                    cmd=request_body+i+1;
                    for(    ; i<len && request_body[i]!='&';i++);
                    request_body[i]=0;

                    for(    ; i<len && request_body[i]!='=';i++);
                    arg1=request_body+i+1;
                    for(    ; i<len && request_body[i]!='&';i++);
                    request_body[i]=0;

                    for(    ; i<len && request_body[i]!='=';i++);
                    arg2=request_body+i+1;
                    for(    ; i<len && request_body[i]!='&';i++);
                    request_body[i]=0;

                    sprintf(command,"%s %s %s > tmpfile.txt",cmd,arg1,arg2);
                    printf("Eseguo il comando %s\n",command);
                    t=system(command);
                    if (t != -1)
                        strcpy(path+1,"tmpfile.txt");
                }
            }
        }
        //Fine delle modifiche

        else if(strncmp(path,"/cgi-bin/",9) == 0){

            sprintf(command,"%s > tmpfile.txt", path+9);
            printf("Eseguo il comando %s\n",command);
            t=system(command);
            if (t != -1)
                strcpy(path+1,"tmpfile.txt");
        }
        if ((fin = fopen(path+1,"rt"))==NULL){
            sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
            write(s2,response,strlen(response));
        }
        else {
            sprintf(response,"HTTP/1.1 200 OK\r\n\r\n");
            write(s2,response,strlen(response));
            while ( (c = fgetc(fin)) != EOF )
                write(s2,&c,1);
            fclose(fin);
        }
        close(s2);
    }
}
