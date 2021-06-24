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
    // dichiaro la dimensione del range 
    int range = 10000;
    // indice ultimo byte letto
    long int index = 0;
    // intervallo ritornato dall'header Content-Range
    long int first = 0;
    long int last = 0;
    long int totlen = 1;
    char * cr;
    // array di puntatori ai diversi buffer in cui è diviso il risultato
    // una risorsa può essere divisa in al massimo 10 000 buffer, di conseguenza
    // si possono richiedere risorse grandi al massimo 10kB x 10kB = 1GB
    char * buffs[10000];

    char * statusline;
    struct sockaddr_in addr;
    int entity_length;
    int i,j,k,s,t;

    char request[5000],response[10000];
    char * entity;

    // aggiorno l'indirizzo target del server
    unsigned char targetip[4] = {  88,80,187,84};
    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){
        tmp=errno;
        perror("Socket fallita");
        printf("i=%d errno=%d\n",i,tmp);
        return 1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int*)targetip;

    if ( -1 == connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
        perror("Connect fallita"); 
    

    for(i=0; last<totlen;i++) {
   
        // modificati path e header Host nella GET
        // aggiunta l'header Range in cui si definisce 
        sprintf(request,"GET /image.jpg HTTP/1.1\r\nHost:88.80.187.84\r\nConnection:keep-alive\r\nRange:bytes=%ld-%ld\r\n\r\n", index, index+range-1);
        int dimension = 0;
        printf("%s", request);

        if ( -1 == write(s,request,strlen(request))){perror("write fallita"); return 1;}
        bzero(h,sizeof(struct header)*100);
        statusline = h[0].n=response;
        for( j=0,k=0; read(s,response+j,1);j++){
            if(response[j]==':' && (h[k].v==0) ){
                response[j]=0;
                h[k].v=response+j+1;
            }
            else if((response[j]=='\n') && (response[j-1]=='\r') ){
                response[j-1]=0;
                if(h[k].n[0]==0) break;
                printf("%s --> %s\n", h[k].n, h[k].v);
                // nella risposta è fornito l'header Content-Range
                if(!strcmp("Content-Range", h[k].n)) {
                    cr = h[k].v+7;
                    printf("%s\n", cr);
             
                }
                h[++k].n=response+j+1;
            }
        }


        // parsing lunghezza del blocco letto
        for(j=0; ; j++) if(*(cr+j)=='-') { *(cr+j)=0; break;}
        first = atoi(cr); 
        cr += j+1;
        
        for(j=0; ; j++) if(*(cr+j)=='/') {*(cr+j)=0; break;}
        last = atoi(cr);
        cr += j+1;
        
        if(i==0){
            for(; ; j++) if(*(cr+j)=='\r') {*(cr+j)=0; break;}
            totlen = atoi(cr);
        }

        printf("i: %d index: %ld first: %ld last: %ld len:%ld\n",i, index, first, last, totlen);
  
        getchar();
        /* NON SERVE PIU' determinare la content-length in questo modo
        entity_length = -1;
        printf("Status line = %s\n",statusline);   
        for(i=1;i<k;i++){
            if(strcmp(h[i].n,"Content-Length")==0){
                entity_length=atoi(h[i].v);
                printf("* (%d) ",entity_length);
            }
            printf("%s ----> %s\n",h[i].n, h[i].v);
        }
        entity_length=22000;
        if(entity_length == -1) entity_length=1000000;
        */ 
        
        // alloca la memoria per il segmento appena letto
        dimension = last - first + 1;
       
        buffs[i] = (char * ) malloc(dimension);
        // copia il segmento nel buffer temporaneo
        for(j=0; (t=read(s,buffs[i]+j,dimension-j))>0;j+=t);
        
        // aggiorna gli indici
        index += dimension; 
    }

    // scrivi sequenzialmente le varie parti per comporre l'immagine finale
    FILE * fp = fopen("imagex.jpg", "w+");
    for(j=0; j<i; j++) {
       fputs(buffs[j], fp); 
    }
    fclose(fp);
}

