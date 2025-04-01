#include <iostream>
#include <cstring>
#include <unistd.h> // for close(server) to close socket 
#include <netinet/in.h> // for sockaddr_in, INADDR_ANY, htons
#include <arpa/inet.h> // for ip adress 

int main() {
    int server = socket(AF_INET, SOCK_STREAM, 0); // 2 1 0 // 8080 type default 
    std::cout << "descriptor: " << server<<"\n";
    if (server < 0) {
        std::cerr << "socket error\n";
        return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET; // type of an IP adress IPv4
    addr.sin_port = htons(8080); // port number :(
    addr.sin_addr.s_addr = INADDR_ANY; 

    bind(server, (sockaddr*)&addr, sizeof(addr)); //socket binding to addr struct
    listen(server, 5);

    std::cout << "[OK] Server is started http://localhost:8080\n";

    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello, world!";

    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);
    char client_ip[INET_ADDRSTRLEN];

    while (true) {

        char temp[] = {127, 0, 0, 1};

        int client = accept(server, (sockaddr*)&clientAddr, &clientSize);


        inet_ntop(AF_INET, temp, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client IP: " << client_ip << "\n";


        send(client, response, std::strlen(response), 0);
     
        close(client);
    }

    close(server);
    return 0;
}
