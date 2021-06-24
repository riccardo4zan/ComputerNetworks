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
} h[100];

int main() {
    // BLOCKED SITES LIST, maximum 500 blocks
    char* blocked[500];
    int num_blocked = 0;
    // parsing the file
    FILE* block_file = fopen("blocked.txt","rt");
    // read each line of the file
    int line_size = 50;
    do {
        char* line = malloc(line_size);
        blocked[num_blocked] = fgets(line, line_size, block_file);
        line[strcspn(line, "\n")] = 0;
    } while(blocked[num_blocked++] != NULL);
    num_blocked--;

    // declare a request, a response and a command buffer
    char request[5000], response[10000], command[500];
    // declare and initialize the socket
    int s =  socket(AF_INET, SOCK_STREAM, 0);
    int sockaddr_length = sizeof(struct sockaddr_in);
    if ( s == -1 ) { 
        // if the initialization of the socket failed
        // print an error and terminate the program
        perror("Socket fallita"); 
        return 1; 
    }
    // declare and initialize the address of this server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;      // address family : internet
    addr.sin_port = htons(42069);   // service port : 42069
    addr.sin_addr.s_addr = 0;       // protocol : TCP
    /*
     REUSE ADDRESS OPTION SETTING --> setsockopt(sockfd, level, optname, optval, optlen)
       this function allows to manipulate options for the socket referred to by the
       file descriptor; parameters description: 
        s : file descriptor associated to the socket
        SOL_SOCKET : allows to manipulate options at the sockets API level
        SO_REUSEADDR : indicates that the rules used in validating addresses supplied in a bind() call should
                       allow reuse for local addresses (allow reuse of same address)
        &yes : pointer to the int variable yes, equals to 1, set the value for the option
        sizeof(int) : specify the length of optval parameter
     */
    int yes = 1; 
    int opt_res = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (opt_res == -1) {
        // if setsockopt failed, terminate the program
        perror("setsockopt fallita"); 
        return 1;
    }
    /*
     BINDING --> bind(sockfd, *addr, addrlen)
       when the socket is created using the socket() function, it exists in a namespace, but has no 
       address assigned to it; in the client the bind is performed automatically by the connect(), whereas
       in this case we need to do it as the service needs to be binded to a specific ADDR:PORT
        s : socket file descriptor
        addr : pointer to a sockaddr struct containing the addresses that we want to bind to this service
        addrlen : length of the address structure
     */
    int bind_res = bind(s, (struct sockaddr *) &addr, sockaddr_length);
    if(bind_res == -1) {
        // binding failed, terminate the program
        perror("bind fallita"); 
        return 1;
    }
    /*
     SERVER LISTENING --> listen(sockfd, backlog)
       marks the socket referred by sockfd as a passive socket, that is a socket that will be used
       to accept incoming connections using accept()
        s : file descriptor
        backlog: maximum length to which the queue of pending requests might grow
     */
    if ( listen(s,5) == -1 ) { perror("Listen Fallita"); return 1; }
    
    // ++++++++++ SERVER LOOPS ++++++++++
    while(1) {
        /*
         HTTP REQUEST ACCEPTING --> accept()
           this function extract the first request in the queue of pending requests and creates a new
           socket for it; the original socket is unaffected by this call; the function returns the file
           descriptor of a new socket, that has been automatically binded to a new address
            s : main socket file descriptor
            addr : pointer to a sockaddr structure
            len : size of the sockaddr structure
         */
        struct sockaddr_in remote_addr;
        int s2 =  accept(s, (struct sockaddr *) &remote_addr, &sockaddr_length);
        // if the creation of the new socket retrived an error, terminate the program
        if ( s2 == -1 ) { perror("Accept Fallita"); return 1;}
        
        // intialize with zeros the structs used to contain the request headers
        bzero(h, 100*sizeof(struct header *));

        // declare and initialize the command line 
        char* commandline = h[0].n = request;
        /*
         REQUEST HEADER PARSING
         */
        int num_head = 0;
        char refer_url[150];
        int redirection = 0;
        for(int j=0; read(s2, request+j, 1); j++) {
            if(request[j]==':' && (h[num_head].v==0)) {
                // header name ended, now read the header value
                request[j] = 0;
                h[num_head].v = request+j+1;
            } else if((request[j]=='\n') && (request[j-1]=='\r')) {
                // header value ended, read the next header or terminate
                request[j-1] = 0;
                if(h[num_head].n[0] == 0) break;
                if(!strcmp(h[num_head].n, "Referer"))
                    sprintf(refer_url, h[num_head].v, strlen(h[num_head].v));
                h[++num_head].n = request+j+1;
            } 
        }
           
        /*
         COMMAND LINE PARSING
            from command line, it is possible to get the request method, path and version             
         */
        char * method, *path, *ver;
        method = commandline;
        // parse the path
        int i;
        for(i=0;commandline[i]!=' ';i++){} commandline[i]=0; path = commandline+i+1;
        // parse the version
        for(i++;commandline[i]!=' ';i++); commandline[i]=0; ver = commandline+i+1;
        for(i++;commandline[i]!='\r';i++); commandline[i]=0;

        // check wheter refer correspond to a blacklisted url
        char re_url[149];
        sprintf(re_url, " http://88.80.187.84:42069");
        strcat(re_url, path);

        //printf("%s\n", re_url);
        for(int i=0; i<num_blocked; i++)
            if(!strcmp(blocked[i], refer_url) && strcmp(refer_url, re_url)) 
                redirection = 1;
        /*
         CGI-BIN : RUN APPLICATIONS outside the web server
         */
        if(strncmp(path,"/cgi-bin/",9) == 0){
            // print the command that has to be run in the command buffer
            // the output of the command is redirected to a "tmpfile.txt"
            // created on the same execution folder of the web server
            
            /*
             PARAMETERS parsing
             */
            struct header parameters[50];
            
            // parse the path until a '?' is found or the path ends
            int par_beg = 0, num_par = 0;
            while( (par_beg < strlen(path)) && path[par_beg]!='?') par_beg++;

            if(par_beg < (int)strlen(path)) {
                // there are parameters
                // set a new termination to the path, replacing the '?'
                int pathlength = strlen(path);
                path[par_beg++] = 0;
                // set the pointer to the first parameter name
                parameters[num_par].n = (char*) path+par_beg;
                // this cicle ends when the end of the path is reached
                // no need to terminate the last param, there is already a final 0
                for(int j=0; j+par_beg < pathlength; j++) {
                    if(path[par_beg+j]=='=') {
                        // end of the parameter name
                        path[par_beg+j] = 0;
                        parameters[num_par].v = (char*) path+par_beg+j+1;
                    } else if(path[par_beg+j]=='&') {
                        // end of the parameter value
                        path[par_beg+j] = 0;
                        parameters[++num_par].n = (char*) path+par_beg+j+1;
                    }
                }
                num_par++;
            }
            
            // build the command, adding to the main command the parameters        
            sprintf(command, "%s", path+9);
            int bytes = strlen(path)-9;

            for(int i=0; i<num_par; i++) {
                sprintf(command+bytes, " %s", parameters[i].n);
                bytes += strlen(parameters[i].n)+1;
                sprintf(command+bytes, " %s", parameters[i].v);
                bytes += strlen(parameters[i].v)+1;
            }
            sprintf(command+bytes, " > tmpfile.txt");

            // execute the command using the system() function
            int command_res = system(command);
            if (command_res != -1)
                // if the command has successfully terminated, replace
                // the path with the tmpfile.txt
                strcpy(path+1,"tmpfile.txt");	
        }
        // return the answer to the client
        FILE* fin;
        if(redirection) {
            sprintf(response,"HTTP/1.1 302 Moved Temporarily\r\nLocation: %s\r\n\r\n", refer_url);
            write(s2,response,strlen(response));
        } else if ((fin = fopen(path+1,"rt"))==NULL) {
            // if the resource does not exists, return a 404 code
            // no response returned
            sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
            write(s2,response,strlen(response));
        } else {
            // write a 200 code into the response buffer
            sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
            // send the code
            write(s2, response, strlen(response));
            // send the content of the file
            char c;
            while ((c = fgetc(fin)) != EOF) write(s2, &c, 1);
            // close the file
            fclose(fin);
        }
        // close the temp socket
        close(s2);
        
    } // while(true)

}
