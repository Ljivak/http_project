// Dmytro Malynyak 

#ifndef Socket_h
#define Socket_h

#include <iostream>
#include <stdio.h>
#include <type_traits>
#include <sys/socket.h>  
#include <netinet/in.h>  //address struct 
#include <unistd.h> // close()
#include <arpa/inet.h>  //inet_ntop


class Socket{
private:
    int sock;
    int connection;
    int listening;
    int backlog;
    sockaddr_in address;

public:
   
    Socket(int domain, int server, int protocol, int port, unsigned int address);
    int establish_connection();
    void start_listening(int backlog); // maximum queue
    int accept_connection();


    void test_connection(int connection) const;
};


#endif