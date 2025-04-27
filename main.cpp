#include "HTTPServer.hpp"
#include <fstream>

std::string loadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
}

int main() {

    HTTPServer server(8080);
    
    // Маршрут для корня отдаёт статический HTML
    server.addRoute("GET", "/test", [](const HTTPRequest& req){
        HTTPResponse res;
        res.headers["Content-Type"] = "text/html";
        res.body = loadFile("www/index.html");
        return res;
    });

    // Любой другой путь — 404
    server.listen(10);
    return 0;
}
