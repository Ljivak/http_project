#pragma once
#include <string>
#include <map>
#include <functional>
#include <iterator>
#include <fstream>
#include "Socket.hpp"


struct HTTPRequest {

    std::string method;
    std::string path;
    std::map<std::string,std::string> headers;
    std::string body;

    static HTTPRequest parse(int client_fd);
};

struct HTTPResponse {

    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string,std::string> headers;
    std::string body;

    std::string serialize() const;
};

class HTTPServer {
    public:
    
        using Handler = std::function<HTTPResponse(const HTTPRequest&)>;
    
        HTTPServer(int port, unsigned int interface = INADDR_ANY);
    
        void addRoute(const std::string& method, const std::string& path, Handler handler);
                  
        void addStaticRoute(const std::string& uri, const std::string& filePath, const std::string& contentType = "text/html");

        void addPostRoute (const std::string& path, Handler h);

        void addStaticPostRoute();

        void addPullRoute (const std::string& path, Handler h);

        void addStaticPullRoute();

        static std::string loadFile(const std::string& path);

        void printResponce(const std::string &out) const;

        void listen(int backlog = 5);
    
    private:
        Socket server_socket;
        std::map<std::pair<std::string,std::string>, Handler> routes;

};

