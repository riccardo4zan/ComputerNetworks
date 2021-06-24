//Client web HTTP 1.0 con lettura dell'header content length

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tmp; //Variabile usata per la gestione degli errori nel socket

//Struttura usata per effettuare il parsing degli header
//n punta il nome dell'header
//v punta il valore dell'header
//nella grammatica gli header sono riportati come coppie name:value
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

    //Buffer per la scrittura della richiesta e per il parsing della response
    char request[5000],response[1000000];
    unsigned char targetip[4] = { 172, 217, 16, 142};
   
    //Apertura del socket
    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){
        tmp=errno;
        perror("Socket fallita");
        printf("i=%d errno=%d\n",i,tmp);
        return 1;
    }
    
    //Imposto il server di destinazione della connessione
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int*)targetip;

    //Tento la connessione
    if ( -1 == connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
        perror("Connect fallita"); 
    
    printf("Socket number: %d\n",s);
   
    //Scrittura della request sul socket
    sprintf(request,"GET / HTTP/1.0\r\n\r\n");  //Attenzione ai DUE CRLF \r\n
    if ( -1 == write(s,request,strlen(request))){
        perror("write fallita"); 
        return 1;
    }

    //PARSING DELLA RISPOSTA HTTP 1.0

    //La status line non segue la struttura standard degli altri headers
    //viene letta dal ciclo for seguente e terminata dal ramo else
    //quando viene letto un CRLF, poi si comincia il parsing degli headers
    statusline = h[0].n = response;

    //Leggo un byte per volta dalla response
    for( j=0,k=0; read(s,response+j,1) ;j++){
        //Se trovo i ':' e il campo v dell'header su cui mi trovo è vuoto allora ho appena
        //finito di leggere un nome (altrimenti potrebbe essere anche un ora:minuti:secondi)
        if(response[j]==':' && (h[k].v==0) ){
            response[j]=0;          //Sostituisco i : con il terminatore di stringa
            h[k].v=response+j+1;    //Comincio a leggere il suo valore
        }
        //Se trovo un CRLF allora sono arrivato alla fine di un header
        else if((response[j]=='\n') && (response[j-1]=='\r') ){
            response[j-1]=0;        //Sostituisco \r con il terminatore di stringa
            if(h[k].n[0]==0) break; //CONDIZIONE DI TERMINAZIONE: QUANDO TROVO DUE CRLF DI FILA
            h[++k].n=response+j+1;  //Comincio a leggere il nome di un altro header
        }
    }	

    printf("Header risposta:\n");
    printf("Status line = %s\n",statusline);   
    for(i=1;i<k;i++)
        printf("%s ----> %s\n",h[i].n, h[i].v);


    //GESTIONE DELL'HEADER CONTENT-LENGTH
    int entitybody_length = -1;

    //Verifico se presente Content-length tra header ricevuti 
    //ed eventualmente estraggo la lunghezza del entity body
    for(i=0;i<k;i++){
        if(strcmp(h[i].n,"Content-Length")==0){
            entitybody_length = atoi(h[i].v); //atoi converte una stringa in un intero
            break;
        }
    }
    printf("\nThe entity body length is  %d bytes \n",entitybody_length);

    //In mancancanza della lunghezza imposto la lunghezza 
    //dell'entity body uguale alla dimensione dello spazio libero nel buffer
    entitybody_length = entitybody_length==-1 ? sizeof(response)-j-1 : entitybody_length; 

    //Lettura contenuto entity body 

    // creo un puntatore per entity body
    char* entitybody = response+j;

    //consumo entity body della risposta tenendo conto dell'eventuale presenza dell header Content-length
    while((t=read(s,&response[j],entitybody_length))>0){
        j+=t;
        entitybody_length-=t;
    }

    //inserisco terminatore alla fine dell'entity body
    response[j] = 0; 

    //Gestisco eventuale errore nella lettura dell'entity body
    if (t == -1){
        perror("Read fallita"); 
        return 1;
    }

    printf("%s",entitybody);
    printf("\n");
}

