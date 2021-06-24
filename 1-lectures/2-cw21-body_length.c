//Variante rispetto al programma 1-cw21-content_length nel quale una volta letto l'header content length, se presente
//viene creato un buffer della dimensione esatta per la lettura dell'entity body
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

int tmp; //Variabile usata per la gestione degli errori nel socket

//Struttura usata per effettuare il parsing degli header
////n punta il nome dell'header
////v punta il valore dell'header
////nella grammatica gli header sono riportati come coppie name:value
struct header{
    char * n;
    char * v;
}h[100];

int main()
{
    //La status line precede gli header, contiene versione del protocollo e codice di risposta
    char * statusline;
    struct sockaddr_in addr;

    //i viene usato come indice generico
    //j tiene conto del numero di byte letti dalla response
    //k viene usata per tenere il conto del numero di headers letti
    //s viene usata per tenere traccia del socket che stiamo usando
    //t viene usata per tenere traccia del numero di byte letti dall'entity body (dopo gli header)
    int i,j,k,s,t;

    char request[5000],response[10000];
    unsigned char targetip[4] = { 172,217,23,110 };

    //Gestione dell'entity con header content-length
    int entity_length;
    char * entity; 

    //Apertura socket
    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){
        tmp=errno;
        perror("Socket fallita");
        printf("i=%d errno=%d\n",i,tmp);
        return 1;
    }

    //Impostazione della connessione
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int*)targetip;

    //Apertura della connessione e gestione di eventuali errori
    if ( -1 == connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
        perror("Connect fallita");

    printf("Socket number: %d\n",s);
    
    //Questo ciclo ha lo scopo di fare più volte la stessa richiesta usando l'header
    //Connection:keep-alive in modo di suggerire al server di non chiudere la connessione
    //dopo aver rispsto
    for(int iter=0;iter<1;iter++){

        //Chiedo a google.com la pagina /pluto (che so non esistere)
        sprintf(request,"GET /pluto HTTP/1.0\r\nConnection:keep-alive\r\n\r\n");
        
        //Gestione di eventuali errori
        if ( -1 == write(s,request,strlen(request))){
            perror("write fallita"); 
            return 1;
        }
        
        //Pongo a 0 l'array di 100 strutture dichiarato sopra il main usato per
        //il parsing degli header, così da non avere refusi di richieste vecchie
        bzero(h,sizeof(struct header)*100);
        
        //La status line è una eccezione rispetto al normale parsing degli headers
        statusline = h[0].n=response;
        
        for( j=0,k=0; read(s,response+j,1);j++){
            if(response[j]==':' && (h[k].v==0) ){
                response[j]=0;
                h[k].v=response+j+1;
            }
            else if((response[j]=='\n') && (response[j-1]=='\r') ){
                response[j-1]=0;
                if(h[k].n[0]==0) break;
                h[++k].n=response+j+1;
            }
        }	
        
        entity_length = -1;
        
        //Printing di status line e headers
        printf("Status line = %s\n",statusline);
        for(i=1;i<k;i++){
            //Se trovo un header che specifica la content length
            //allora salvo il valore di questo
            if(strcmp(h[i].n,"Content-Length")==0){
                entity_length=atoi(h[i].v);
                printf("* (%d) ",entity_length);
            }
            printf("%s ----> %s\n",h[i].n, h[i].v);
        }
        
        //Se non ho trovato un header che mi specificasse
        //la lunghezza dell'entity body allora la imposto
        //ad una dimensione a piacere sperando sia abbastanza
        if(entity_length == -1) entity_length=1000000;
        
        //Alloco lo spazio e lo faccio puntare a entity
        entity = (char * ) malloc(entity_length);
        
        //Cerco di leggere con la read il massimo numero di byte
        //possibili e incremento t con il numero di byte letti.
        //j è un riferimento a quante posizione di entity sono usate
        for(j=0; (t=read(s,entity+j,entity_length-j))>0;j+=t);
        
        if (t == -1) {
            perror("Read fallita"); 
            return 1;
        }
        
        printf("Numero di byte di cui è composto entity_body j= %d\n",j);

        //Printing carattere per carattere (potevo fare anche con un fine stringa)
        for(i=0;i<j;i++) 
            printf("%c",entity[i]);
    }
}

