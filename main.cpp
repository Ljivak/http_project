#include "HTTPServer.hpp"
#include <fstream>


int main() {

    HTTPServer server(8080); // create Socket (socket, connection, listening, backlog, address)
    server.addStaticRoute("/", "index.html"); //
    server.listen();
    return 0;
}
