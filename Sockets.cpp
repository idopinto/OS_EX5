//
// Created by ilana315061945 on 10/06/2022.
//

#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <cstring>
#include <netdb.h>
#include <cstdio>
#include <string>

#define MAX_CLIENTS 2
#define MAXHOSTNAME 256

#define BUFFER_SIZE 256
//#define SERVER_PORT 80

int establish_server_socket(unsigned short portnum) {
    char myname[MAXHOSTNAME+1];
    int server_socket;
    struct sockaddr_in server_address;
    struct hostent *hp;


    //hostnet initialization
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);
    if (hp == NULL)
        return(-1);
//    printf("Host name : %s\n", hp->h_name);
//    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)hp->h_addr)));

    //sockaddrr_in initlization
    bzero(&server_address, sizeof (server_address));
//    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = hp->h_addrtype;

    /* this is our host address */
    memcpy(&server_address.sin_addr, hp->h_addr, hp->h_length);

    /* this is our port number */
    server_address.sin_port= htons(portnum);

    /* create socket */
    if ((server_socket= socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return(-1);

//    printf("binding socket to server address... \n");
    if (bind(server_socket , (struct sockaddr *)&server_address , sizeof(struct sockaddr_in)) < 0) {
        close(server_socket);
        return(-1);
    }
//    printf("binding SUCCESS \n");
//
//    printf("start listening to maximum 5 queued connects \n");

    listen(server_socket, MAX_CLIENTS); /* max # of queued connects */
    return server_socket;
}

int get_connection(int s) {
    int new_socket; /* socket of connection */
    if ((new_socket = accept(s,NULL,NULL)) < 0)
        return -1;
//    printf("connection request accepted!!!!!!!!!!!\n");
    return new_socket;
}

int call_socket(char *hostname, unsigned short portnum) {

    struct sockaddr_in server_address;
    struct hostent *hp;
    int client_socket;

    if ((hp= gethostbyname (hostname)) == NULL) {
        return(-1);
    }
//    printf("Host2 name : %s\n", hp->h_name);
//    printf("IP2 Address : %s\n",inet_ntoa(*((struct in_addr *)hp->h_addr)));

    memset(&server_address, 0, sizeof(server_address));
    memcpy((char *)&server_address.sin_addr , hp->h_addr ,
           hp->h_length);
    server_address.sin_family = hp->h_addrtype;
    server_address.sin_port = htons((u_short)portnum);

    if ((client_socket = socket(hp->h_addrtype,SOCK_STREAM, 0)) < 0) {
        return(-1);
    }
//    printf("Client 1 trying to connect with server host %s on port %d\n", hp->h_name, portnum);
    if (connect(client_socket, (struct sockaddr *)&server_address , sizeof(server_address)) < 0) {
        close(client_socket);
        return(-1);
    }
//    printf("SUCCESS !!! Connection established \n");

    return client_socket;
}

int read_data(int server_socket, char *buf, int buffer_length) {
    int bcount = 0;       /* counts bytes read */
    int br = 0;            /* bytes read this pass */
    while (bcount < buffer_length) { /* loop until full buffer */
        br = read(server_socket, buf, buffer_length - bcount);
        if (br > 0){
            bcount += br;
            buf += br;
        }
        if (br < 1) {
            return -1;
        }
    }
    return bcount;
}

int main(int argc, char* argv[]){
    char command_to_execute[BUFFER_SIZE];  // to send
    char recv_command_to_execute[BUFFER_SIZE];  // receive
    int command_length;
    int server_fd,new_socket_fd;
    char hostname[MAXHOSTNAME + 1];
    if(argc < 3){
        // error
    }
    if(argc == 3){
        if(strcmp(argv[1],"server") != 0){
            // error
        }
        // server side
        unsigned int server_port_num = (strtoul(argv[2], nullptr, 10));
        server_fd = establish_server_socket(server_port_num);
        while(true){
            new_socket_fd = get_connection(server_fd);
            read_data(new_socket_fd,recv_command_to_execute,BUFFER_SIZE);
            // run the command that client sent
            system(recv_command_to_execute);
            close(new_socket_fd);
            bzero(recv_command_to_execute, strlen(recv_command_to_execute));
        }

    }
    else if (argc >= 4){
        if(strcmp(argv[1],"client") != 0){
            // error
        }
        // client side
        gethostname(hostname, MAXHOSTNAME);
        unsigned int client_port_num = (strtoul(argv[2],nullptr,10));
        int client_socket = call_socket(hostname, client_port_num);
        if(client_socket == -1){
            //error
        }

        // send argv[2]
        int counter = 3;
        std::string command;
        while(counter < argc){
            command += argv[counter];
            counter++;
            if(counter != argc){
                command += " ";
            }
        }
        sprintf(command_to_execute,"%s", const_cast<char*>(command.c_str()));
        command_length = (int) strlen(command_to_execute);
//        printf("command to execute: %s\n",command_to_execute);
        if(write(client_socket,command_to_execute,command_length)!=command_length){
            printf("error\n");
        }
    }
}