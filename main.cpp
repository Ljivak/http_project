#include "HTTPServer.hpp"
#include <fstream>


int main() {

    HTTPServer server(8080); // create Socket (socket, connection, listening, backlog, address)
    server.addStaticRoute("/", "index.html");

    server.addStaticPostRoute();
    server.addStaticPutRoute();
    // server.addEchoRoutes();

    server.listen();
    return 0;
}
