#include "Socket.hpp"
#include "Socket.cpp"

int main(){


    Socket sock = Socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
    sock.establish_connection();
    sock.start_listening(10);
    
    while(true){
        sock.accept_connection();
    }
    

}