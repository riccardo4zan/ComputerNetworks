#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

struct hostent * he;
struct sockaddr_in local,remote,server;
char request[2000],response[2000],request2[2000],response2[2000];
char * method, *path, *version, *host, *scheme, *resource,*port;
struct headers {
    char *n;
    char *v;
}h[30];

/*
 Gestione opposta della modalità di connessione del proxy

 La scelta che si è deciso di implementare è stata quella di mantenere una connessione keep-alive
 dal lato client, mentre chiedere sempre la chiusura della connessione dal lato server. Questo,
 oltre ad essere più logico dal punto di vista pratico (il client verosimilmente manderà diverse
 richieste consecutive al proxy, mentre le richieste che il proxy manderà non saranno necessariamente
 tutte allo stesso server e, soprattutto, se si mantenesse una connessione persistente tra client e
 server avrebbe più senso mantenere una sola connessione persistente per tutti i client collegati al
 proxy, molto più difficile da implementare).

 Modifiche che dovranno essere implementate:
    + cambiamento gestione connessione verso il client, che non dovrà più essere terminata una volta
      finita la gestione della richiesta ma, al contrario, dovrà essere in grado di gestire più
      richieste da parte del client --> aggiunta ciclo sulla gestione delle richieste
    + aggiunta header Connection:close nella richiesta mandata al server per essere sicuri che la
      connessione sia terminata di comune accordo da entrambi (già presente)
    + controllo e sostituzione dell'header Connection inserito dal server
 */
int main()
{
    FILE *f;
    char command[100];
    int i,s,t,s2,s3,n,len,c,yes=1,j,k,pid; 
    s = socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1) { perror("Socket Fallita\n"); return 1;}
    local.sin_family=AF_INET;
    local.sin_port = htons(42069);
    local.sin_addr.s_addr = 0;
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    t = bind(s,(struct sockaddr *) &local, sizeof(struct sockaddr_in));
    if ( t == -1) { perror("Bind Fallita \n"); return 1;}
    t = listen(s,10);
    if ( t == -1) { perror("Listen Fallita \n"); return 1;}
    while( 1 ){
        f = NULL; 
        remote.sin_family=AF_INET;
        len = sizeof(struct sockaddr_in);
        s2 = accept(s,(struct sockaddr *) &remote, &len);
        if(fork()) continue; 
        if (s2 == -1) {perror("Accept Fallita\n"); return 1;}
        j=0;k=0;
        bzero(h,30*sizeof(struct header *));
        // inizio gestione keep-alive
        do {

            h[k].n = request;
            while(read(s2,request+j,1)){
                if((request[j]=='\n') && (request[j-1]=='\r')){
                    request[j-1]=0;
                    if(h[k].n[0]==0) break;
                    h[++k].n=request+j+1;
                }
                if(request[j]==':' && (h[k].v==0) &&k ){
                    request[j]=0;
                    h[k].v=request+j+1;
                }
                j++;
            }
            printf("%s",request);
            method = request;
            for(i=0;(i<2000) && (request[i]!=' ');i++); request[i]=0;
            path = request+i+1;
            for(   ;(i<2000) && (request[i]!=' ');i++); request[i]=0;
            version = request+i+1;

            printf("Method = %s, path = %s , version = %s\n",method,path,version);	
            if(!strcmp("GET",method)){ 
                //  http://www.google.com/path
                scheme=path;
                for(i=0;path[i]!=':';i++); path[i]=0;
                host=path+i+3; 
                for(i=i+3;path[i]!='/';i++); path[i]=0;
                resource=path+i+1;
                printf("Scheme=%s, host=%s, resource = %s\n", scheme,host,resource);
                he = gethostbyname(host);
                if (he == NULL) { printf("Gethostbyname Fallita\n"); return 1;}
                printf("Server address = %u.%u.%u.%u\n", (unsigned char ) he->h_addr[0],(unsigned char ) he->h_addr[1],(unsigned char ) he->h_addr[2],(unsigned char ) he->h_addr[3]); 			
                s3=socket(AF_INET,SOCK_STREAM,0);
                if(s3==-1){perror("Socket to server fallita"); return 1;}
                server.sin_family=AF_INET;
                server.sin_port=htons(80);
                server.sin_addr.s_addr=*(unsigned int*) he->h_addr;			
                t=connect(s3,(struct sockaddr *)&server,sizeof(struct sockaddr_in));		
                if(t==-1){perror("Connect to server fallita"); return 1;}
                sprintf(request2,"GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n",resource,host);
                printf("%s", request2);
                
                write(s3,request2,strlen(request2));

                // parse server response and send back to the client
                // if an header Connection is found, change the value to keep-alive
                char srv_resp[5000];
                j=0, k=0;
                bzero(h,30*sizeof(struct header *));
                h[k].n = srv_resp;
                int con_len = 0;
                while(read(s3,srv_resp+j,1)){
                    if((srv_resp[j]=='\n') && (srv_resp[j-1]=='\r')){
                        srv_resp[j-1]=0;
                        if(h[k].n[0]==0) break;
                        h[++k].n=srv_resp+j+1;
                    }
                    if(srv_resp[j]==':' && (h[k].v==0) &&k ){
                        srv_resp[j]=0;
                        h[k].v=srv_resp+j+1;
                       
                    }
                    j++;
                } 
               
                
                write(s2,h[0].n, strlen(h[0].n));
                printf("%s", h[0].n);
                char hea[40];

                for(i=1;i<k;i++){
                    if(!strcmp("Connection", h[i].n)) h[i].v=" keep-alive";
                    printf("\r\n%s:%s", h[i].n, h[i].v);
                    sprintf(hea,"\r\n%s:%s",h[i].n, h[i].v);
                    write(s2,hea,strlen(hea));
                }
                sprintf(hea,"\r\nKeep-Alive: timeout=5, max=20");
                write(s2,hea,strlen(hea));
                sprintf(hea,"\r\n\r\n");
                write(s2,hea,4);
                printf("\r\n\r\n");

                // read the entity body
                while(t=read(s3,response2,2000)) write(s2,response2,t);	
                shutdown(s3,SHUT_RDWR);
                close(s3);
            }
            bzero(h,30*sizeof(struct header *));
            char ack[3];
            strcmp(ack, "OK");
            while(!(t=read(s2, request, 1))) {       
                if(write(s2,ack,2)) {
                    printf("\n++++++++++++++++++++++++++\nShut down connection!!!!\n++++++++++++++++++++++++++\n");
                    shutdown(s2,SHUT_RDWR);
		            close(s2);	
		            exit(0);
                }
            }
            printf("\n**********\nRimango nella connessione!\n***********\n\n");
            j=1, k=0;
        } while(1);
        // fine gestione keep-alive
    }
}
