//Client Web basic, effettua richieste utilizzando il protocollo HTTP 0.9

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

int tmp; //variabile usata solo per mostrare errore se socket fallisce
int main()
{
    struct sockaddr_in addr;
    int i,s,t;
    char request[5000],response[1000000];
    unsigned char targetip[4] = {142,250,181,227};      //nslookup google.com 

    //Apertura del socket
    s =  socket(AF_INET, SOCK_STREAM, 0); 
    if ( s == -1 ){
        tmp=errno;
        perror("Socket fallita");
        printf("i=%d errno=%d\n",i,tmp);
        return 1;
    }

    //Compilazione della struttura per la connessione
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);                          //porta a cui collegarsi
    addr.sin_addr.s_addr = *(unsigned int*)targetip;

    //Effettuazione della connessione e controllo errori
    if ( -1 == connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in))) 
        perror("Connect fallita"); 
    
    printf("Socket number: %d\n",s);

    //Scruttra della richiesta sul buffer request
    sprintf(request,"GET / \r\n");
    
    //Scrittura della request sul socket
    if ( -1 == write(s,request,strlen(request))){
        perror("Write fallita"); 
        return 1;
    }
    
    //Provo a leggere la risposta del server dal socket s
    //la funzione read restituisce il numero di byte (caratteri) letti
    while((t=read(s,response,999999))>0){
        //for(i=0;i<t;i++) printf("%c",response[i]);    //printo tutti i caratteri nella response
        response[t]=0;                                  //inserisco il terminatore di stringa
        //printf("%s",response);                          //printo direttamente la response con terminatore
        printf("\n***** Numero di byte letti t = %d *****\n",t);
    }

    //Se alla variabile t viene assegnato il valore -1 allora la read ha fallito
    if ( t == -1) { 
        perror("Read fallita"); 
        return 1;
    }
}

