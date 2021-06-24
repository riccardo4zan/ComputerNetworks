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
   
    //i viene usato come indice generico
    //j viene usato come indice per il buffer request
    //k contiene il numero di headers della request
    //s socket su cui il programma è in attesa di connessioni
    //t utilizzata per settare le
    //s2 socket a cui il client si collega (socket restituito dalla accept)
    //len lunghezza della HTTP request mandata dal client
    int i,j,k,s,t,s2,len;
    
    char command[100];  //Contiene il comando che il server dovrà eseguire
   
    int c;              //Variabile utilizzata per la lettura da file
    
    FILE *fin;
    
    int yes=1;          //Valore booleano per riusare la coppia IP:Port
    
    char *commandline;  //Prima linea di unq richiesta HTTP
    
    char *method, *path, *ver;  //3 componenti della commandline HTTP da parsare
    
    char request[5000],response[10000];
    
    //Apertura di un socket
    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){ 
        perror("Socket fallita"); 
        return 1; 
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8777);   //Porta sulla quale il server ascolta
    addr.sin_addr.s_addr = 0;       //Utilizza l'indirizzo IP della macchina, 0 per rispondere a qualsiasi indirizzo della macchina
    t = setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)); //Permette il riutilizzo della stessa coppia IP:Port per il server
    
    if ( t==-1 ){
        perror("setsockopt fallita"); 
        return 1;
    }
   
    //La funzione bind associa il socket ottenuto dal SO con le impostazioni specificate da riga 50 a 53
    if ( bind(s,(struct sockaddr *)&addr,sizeof(struct sockaddr_in)) == -1 ) {
        perror("bind fallita"); 
        return 1;
    }
   
    //s è il socket su cui mettersi in ASCOLTO mentre 255 è il massimo numero di connessioni in coda
    //la listen METTE IN CODA LE CONNESSIONI per far fronte ai burst di richieste
    if ( listen(s,225) == -1 ) { 
        perror("Listen Fallita"); 
        return 1; 
    }
    
    //Lunghezza della struttura sockaddr, usata poi nella accept perchè il SO
    //potrebbe modificare la lunghezza della struttura dopo aver accettato la connessione
    len = sizeof(struct sockaddr_in);
   
    //Il server è sempre in ascolto
    while(1){
        //La ACCEPT effettua lo scodamento delle connessioni, messe in coda dalla ACCEPT
        //La ACCEPT crea un nuovo socket e RESTITUISCE PORTA E INDIRIZZO di chi
        //ha aperto la connessione (nella struttura remote_addr)
        s2 =  accept(s, (struct sockaddr *)&remote_addr,&len);
        if ( s2 == -1 ) { 
            perror("Accept Fallita"); 
            return 1;
        }
       
        //Pulisco il buffer per il parsing degli headers
        bzero(h,100*sizeof(struct header *));
        
        //Gli header delle richieste sono uguali a quelli delle risposte
        commandline = h[0].n=request;
       
        //Parsing degli header
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
        
        //Parsing della command line, i token sono separati da spazi
        method = commandline;
        for(i=0;commandline[i]!=' ';i++); 
        commandline[i]=0; 
        
        path = commandline+i+1;
        //Il primo i++ è per saltare lo spazio dopo method
        for(i++;commandline[i]!=' ';i++); 
        commandline[i]=0;

        ver = commandline+i+1;
        //il terminatore NULL dopo il token versione è già stato messo dal parser delle righe/headers

        //Printo la request line
        printf("method=%s path=%s ver=%s\n",method,path,ver);
       
        printf("Fine richiesta\n");

        //IMPLEMENTAZIONE DEL CONTROLLO DEL PATH DELLA REQUEST

        //Common Gateway Interface usata per il passaggio dei parametri
        //il terzo parametero si strncmp corrisponde alla lunghezza della
        //stringa /cgi/bin/ path è il parametro parsato sopra
        if(strncmp(path,"/cgi-bin/",9) == 0){

            //Metto in un file tmpfile.txt l'output del programma
            //path+9 è il comando dopo /cgi-bin 
            //esempio (/cgi-bin/ls)
            
            //sprintf effettua addizioni di stringhe.
            //In questo caso fa puntare a command il buffer composto da
            //path+9 addizionata con > tmpfile.txt
            sprintf(command,"%s > tmpfile.txt", path+9);
            printf("Eseguo il comando %s\n",command);

            //system attiva una shell e manda il comando
            //system ritorna -1 nel caso l'esecuzione fallisca
            t = system(command);
            if (t != -1)
                //Dopo aver salvato la risposta da dare nel file tmpfie.txt
                //modifica la richiesta in modo che il blocco do codice sotto
                //butti fuori tmpfile.txt come file di output
                strcpy(path+1,"tmpfile.txt");	
        }
       
        //Cerca di aprire il file specificato in path
        if ((fin = fopen(path+1,"rt"))==NULL){
            //Se è nullo allora 404 NOT FOUND
            sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
            write(s2,response,strlen(response));
        }
        else {

            //Altrimenti trovato, scrivi il codice 200 sul socket
            sprintf(response,"HTTP/1.1 200 OK\r\n\r\n");
            write(s2,response,strlen(response));

            //Leggi carattere per carattere il file da restituire 
            //e scrivilu sul buffer di risposta
            while ( (c = fgetc(fin)) != EOF ) 
                write(s2,&c,1);
            fclose(fin);	
        }
        
        //Chiude il socket
        close(s2);
    }
}

