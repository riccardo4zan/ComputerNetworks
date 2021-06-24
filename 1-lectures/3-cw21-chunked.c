//Implementazione della gestione di Chunked-Transfer-Encoding
//https://datatracker.ietf.org/doc/html/rfc2616#section-3.6.1
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

    //Chiedo a google.com la pagina /
    sprintf(request,"GET / HTTP/1.1\r\n\r\n");

    //Gestione di eventuali errori
    if ( -1 == write(s,request,strlen(request))){
        perror("write fallita"); 
        return 1;
    }

    //La status line è una eccezione rispetto al normale parsing degli headers
    statusline = h[0].n=response;

    for( j=0,k=0; read(s,response+j,1) ;j++ ){
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

    //Printing di status line e headers
    printf("Status line = %s\n",statusline);

    //Mostro gli headers
    for(i=1;i<k;i++)
        printf("%s ----> %s\n",h[i].n, h[i].v);

    //INIZIO DELLE MODIFICHE

    //Cerco se è presente un header che specifica la codifica chunked
    int is_chunked=0;
    for(i=1;i<k;i++)
        if( strcmp(h[i].n,"Transfer-Encoding")==0 && strcmp(h[i].v," chunked")==0 )
            is_chunked=1;
    if(is_chunked) printf("\n***** CHUNKED *****\n");
    else printf("\n***** NOT CHUNKED *****\n");

    if(is_chunked){

        int entity_i = 0;   //Numero di bytes usati nel buffer entity_body
        char* entity = (char*) malloc(100000);

        //Leggo nel buffer read i caratteri byte per byte cercando la prima dimensione
        int chunk_i=0;
        char* chunk = (char*)malloc(100);   //buffer temporaneo per la lettura dei chunk

        int chunk_size;         //Variabile destinata a contenere la dimensione dei chunk

        do{

            chunk_i=0;          //Azzeramento dell'indice
            bzero(chunk,100);   //Azzeramento del buffer
            
            //Leggo dal buffer in cerca di una dimensione del chunk
            while(read(s,chunk+chunk_i,1)>0){
                if(chunk[chunk_i]=='\n' && chunk[chunk_i-1]=='\r'){
                    chunk[chunk_i-1]=0;    //Terminatore di stringa
                    break;
                }
                chunk_i++;
            }
            chunk_size = strtol(chunk, NULL, 16);

            printf("\n*****Chunk size %d *****\n",chunk_size);

            //Leggi chunk_size byte e mettili in entity_body
            int acc=0;  //Accumulatore momentaneo per tenere conto di quanto sto leggendo
            while( (t=read(s,entity+entity_i,1))>0 && acc<chunk_size ){
                acc+=t;
                entity_i+=t;
            }

        }while(chunk_size>0);

        //Terminatore di stringa
        entity[entity_i]=0;

        printf("%s\n",entity);
    }

    //FINE DELLE MODIFICHE

        /*
        //Gestione dell'entity con header content-length
        int entity_length = -1;
        char* entity; 

        for(i=1;i<k;i++){
        //Se trovo un header che specifica la content length salvo il valore
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
        */

}
