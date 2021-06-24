/*
   Authentication con cookies

   Al fine di implementare quanto richiesto dalla consegna, sarà necessario definire:
   + controllo di authentication quando cw richiede la pagina file1.html; per fare
   questo, ci si basa su di un Basic authentication scheme, per cui il client fornisce
   mediante l'header Authorization una stringa che comprende lo schema utilizzato e
   una stringa "username:pwd" codificata mediante base64
   + se l'authentication fornita dal client non è valida o se il client tenta
   accedere alla risorsa senza autenticarsi, 'sfidalo' con una
   WWW-Authenticate
   + se l'authentication da parte del cw va a buon fine, ossia se la coppia user:pwd è
   riconosciuta da sw, il server deve ritornare un cookie (generato a partire dall'ora corrente)
   mediante l'header Set-Cookie: SID=...
   + il client dovrà specificare questo cookie al sw per accedere alla risorsa
   file2.html mediante l'header Cookie
 */

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int tmp;

struct header{
    char * n;
    char * v;
}h[100];

int main()
{
    // hard-coded b64 code for 'user:password'
    // in un sistema più elaborato, il salvataggio di user e password dovrebbe
    // essere implementato mediante l'uso di un database
    char access[20] = "YWRtaW46YWRtaW4=";
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
    addr.sin_port = htons(42069);
    addr.sin_addr.s_addr = 0;
    t= setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    if (t==-1){perror("setsockopt fallita"); return 1;}
    if ( bind(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {perror("bind fallita"); return 1;}
    if ( listen(s,225) == -1 ) { perror("Listen Fallita"); return 1; }
    len = sizeof(struct sockaddr_in);
 
    while(1){
        char * cook = NULL;
        char toadd[50] = "";
        s2 =  accept(s, (struct sockaddr *)&remote_addr,&len);
        if ( s2 == -1 ) { perror("Accept Fallita"); return 1;}
        bzero(h,100*sizeof(struct header *));
        commandline = h[0].n=request;
        // authorization fornita, altrimenti NULL
        int not_auth = 1;
        for( j=0,k=0; read(s2,request+j,1);j++){
            if(request[j]==':' && (h[k].v==0) ){
                request[j]=0;
                h[k].v=request+j+1;
            }
            else if((request[j]=='\n') && (request[j-1]=='\r') ){
                request[j-1]=0;
                // salvataggio contenuto header Authorization, se presente
                if(!strcmp("Authorization", h[k].n)) {
                    not_auth = strcmp(h[k].v+7, access);
                }
                // check header cookie
                if(!strcmp("Cookie", h[k].n)) {
                    cook = h[k].v+5;
                }
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

        printf("method=%s path=%s ver=%s\n",method,path,ver);
        if(strncmp(path,"/cgi-bin/",9) == 0){
            sprintf(command,"%s > tmpfile.txt", path+9);
            printf("Eseguo il comando %s\n",command);
            t=system(command);
            if (t != -1)
                strcpy(path+1,"tmpfile.txt");	
        }
        if(!strcmp(path, "/file2.html")) {
            // apri il file dei cookies
            FILE * cofile = fopen("cookies.txt", "a+");
            char line[20];
            int match = 0;
            if(cook!= NULL && cofile!=NULL) {
                while(fgets(cook, 19, cofile)) {
                    if(!strcmp(cook,line)) {
                        match = 1;
            
                        break;
                    }
                }
            }
            if(!match) {
                // se non si possiede l'autorizzazione, rispondi con il
                // corrispondente messaggio
                sprintf(response,"HTTP/1.1 401 Forbidden\r\nContent-Length:0\r\n\r\n");
                write(s2,response,strlen(response));
                close(s2);
                continue;
            }
        }

        if ((fin = fopen(path+1,"rt"))==NULL){
            sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
            write(s2,response,strlen(response));
        } else if(!strcmp(path, "/file1.html") && not_auth) {
            // il client ha richiesto la pagina di authenticazione senza fornire
            // un header Authorization o ha fornito un'auth sbagliata, richiedi
            // l'accesso allo stesso
            sprintf(response,"HTTP/1.1 401 Forbidden\r\nWWW-Authenticate: Basic realm=\"autenticazione\"\r\n\r\n");
            write(s2,response,strlen(response));
        }         else if(!strcmp("/file1.html",path) && cook==NULL) {
            // se il client ha fornito una corretta autenticazione ed è passato
            // per la pagina file1.html, invia il cookie
            // il cookie è ottenuto dal system time
            time_t cookie = time(0);
            char cs[20];
            sprintf(cs, "%ld\n", cookie);
            // salva il cookie in un file che contiene tutti i cookie
            FILE * cofile = fopen("cookies.txt", "a+");
            fputs(cs, cofile);
            fclose(cofile);

            // aggiungi il cookie alla risposta
            sprintf(response,"HTTP/1.1 200 OK\r\nSet-Cookie: SID=%ld\r\n\r\n", cookie);
            write(s2,response,strlen(response));
            while ( (c = fgetc(fin)) != EOF ) 
                write(s2,&c,1);
            fclose(fin);	
        }
        else {
            sprintf(response,"HTTP/1.1 200 OK\r\n%s\r\n", toadd);
            write(s2,response,strlen(response));
            while ( (c = fgetc(fin)) != EOF ) 
                write(s2,&c,1);
            fclose(fin);	
        }

        close(s2);
        printf("\n");
    }
}

