#include "Socket.hpp"

Socket::Socket(const int domain, const int server, const int protocol, int port, unsigned int interface){

    // define address structure 
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(interface);

    // make connection with socket
    sock = socket(domain, server, protocol);
    test_connection(sock);

}

int Socket::establish_connection(){
    connection = bind(sock, (struct sockaddr *)&address, sizeof(address) );
    test_connection(connection);
    return connection;
}

void Socket::start_listening(int backlog){
    this->backlog = backlog;
    listening = listen(sock, backlog);
    test_connection(listening);
}

int Socket::accept_connection() {
    socklen_t client_len = sizeof(address);
    const int client_socket = accept(sock, (struct sockaddr *)&address, &client_len);
    test_connection(client_socket);
    char client_ip[INET_ADDRSTRLEN]; // length of IPv4 : 255.255.255.255
    inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
    const int client_port = ntohs(address.sin_port);

    std::cout << "connection: IP = " << client_ip << ", port: " << client_port << '\n';

    return client_socket;
}

 
void Socket::test_connection(int connection) const {
    if(connection<0) {
        std::cout << "Failed to connect..." << '\n';
        exit(EXIT_FAILURE);
    }
}
