#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
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
    char * statusline;
    struct sockaddr_in addr;
    int entity_length;
    int i,j,k,s,t;
    char request[5000],response[10000];
    char * entity;
    int max_for = 5;

    //unsigned char targetip[4] = {35,186,238,101};
    unsigned char targetip[4] = {46,37,17,205};

    s =  socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ){
        tmp=errno;
        perror("Socket fallita");
        printf("i=%d errno=%d\n",i,tmp);
        return 1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned int*)targetip; // <indirizzo ip del server 216.58.213.100 >

    if ( -1 == connect(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
        perror("Connect fallita"); 
    int is_chunked = 0;
    for(int iter=0;iter<1;iter++){
        sprintf(request,"TRACE / HTTP/1.1\r\nHost:www.radioamatori.it\r\nMax-Forwards:%d\r\nConnection:close\r\n\r\n", max_for);
        //sprintf(request,"TRACE / HTTP/1.1\r\nHost:www.webtrace.com\r\nMax-Forwards:0\r\nConnection:close\r\n\r\n");
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
                h[++k].n=response+j+1;
            }
        }	

        entity_length = -1;
        for(i=1;i<k;i++){
            if(strcmp(h[i].n,"Content-Length")==0){
                entity_length=atoi(h[i].v);
                printf("* (%d) ",entity_length);
            }
        }
        entity_length=22000;
        if(entity_length == -1) entity_length=1000000;
        entity = (char * ) malloc(entity_length);
        for(j=0; (t=read(s,entity+j,entity_length-j))>0;j+=t);
        //if ( t == -1) { perror("Read fallita"); return 1;}
        
        char dimc[50];
        dimc[0]='0';
        dimc[1]='x';
        int numc;
        for(numc=0;*(entity+numc)!='\r';numc++);
      
        strncpy(dimc+2,entity, numc);

        int size = strtol(dimc, NULL, 16);
        char * body = (char*) malloc(size);

        for(int i=0;i<size;i++) body[i]=entity[i+numc+2];
        k=0;
        bzero(h,sizeof(struct header)*100);
        h[k].n=body;
        for(j=0; ;j++){

            if(body[j]==':' && (h[k].v==0) ){
                body[j]=0;
                h[k].v=body+j+1;
            }
            else if((body[j]=='\n') && (body[j-1]=='\r') ){
                body[j-1]=0;
                if(h[k].n[0]==0) break;   
                
                if(!strcmp(h[k].n, "Max-Forwards")) { 
                    int returned = atoi(h[k].v);
                    if(returned==max_for) printf("Nessun proxy tra client e server!\n");
                    else printf("Proxy tra client e server rilevato!\n");
                }
                h[++k].n=body+j+1;

            }
        }	
        //printf("%s\n", entity);
        //for(i=0;i<j;i++) printf("%c",entity[i]);
    }
}
