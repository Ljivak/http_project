#include "HTTPServer.hpp"
#include <sstream>
#include <iostream>
#include <iterator>

HTTPRequest HTTPRequest::parse(int client_fd) {

    HTTPRequest req;
    std::string line;
    std::stringstream ss;

    // Считать до разделителя заголовков "\r\n\r\n"
    size_t total = 0;
    while (true) {
        char buf[1024];
        int n = recv(client_fd, buf, sizeof(buf), 0); //длина запроса
        if (n <= 0) break;
        ss.write(buf, n);
        total += n;
        if (ss.str().find("\r\n\r\n") != std::string::npos)
            break;
    }
    std::istringstream stream(ss.str());

    // Первая строка: метод, путь, версия
    stream >> req.method >> req.path;
    std::getline(stream, line); // оставляем версию

    // Заголовки
    while (std::getline(stream, line) && line != "\r") {
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string name = line.substr(0,pos);
            std::string value = line.substr(pos+1);
            // убрать \r
            if (!value.empty() && value.back() == '\r')
                value.pop_back();
            req.headers[name] = value;
        }
    }
    auto itLen = req.headers.find("Content-Length");
    if (itLen != req.headers.end()) {
        size_t want = std::stoul(itLen->second);
        std::string bodyBuf(want, '\0');
        size_t have = 0;

        std::string rest = ss.str();
        size_t hdrEnd = rest.find("\r\n\r\n") + 4;
        if (rest.size() > hdrEnd) {
            size_t copy = std::min(want, rest.size() - hdrEnd);
            bodyBuf.replace(0, copy, rest.substr(hdrEnd, copy));
            have = copy;
        }

        while (have < want) {                           // дочитываем из сокета
            int n = recv(client_fd, &bodyBuf[have], want - have, 0);
            if (n <= 0) break;
            have += n;
        }
        req.body = bodyBuf;
    }


    return req;
}

// HTTPResponse::serialize
std::string HTTPResponse::serialize() const {
    std::ostringstream out;
    out << "HTTP/1.1 " << this->status_code << " " << this->status_text << "\r\n";
    for (auto& [k,v] : this->headers) {
        out << k << ": " << v << "\r\n";
    }
    out << "Content-Length: " << this->body.size() << "\r\n";
    out << "Connection: close\r\n";
    out << "\r\n";
    out << body;
    return out.str();
}

// Конструктор HTTPServer
HTTPServer::HTTPServer(const int port, const unsigned int interface)
    : server_socket(AF_INET, SOCK_STREAM, 0, port, interface)
{
    server_socket.establish_connection();
}

// listen
void HTTPServer::listen(int backlog) {
    server_socket.start_listening(backlog);
    std::cout << "HTTPServer listening..." << std::endl;

    while (true) {

        // client_fd := client file descriptor (какой канал/сокет)
        int client_fd = server_socket.accept_connection();
        // Парсинг запроса
        HTTPRequest request = HTTPRequest::parse(client_fd);

        // Поиск обработчика
        Handler handler = nullptr;
        auto check_handler = routes.find({request.method, request.path});
        if (check_handler != routes.end()) {
            handler = check_handler->second;
        }

        HTTPResponse responce;
        if (handler) {
            responce = handler(request);
        } else {
            responce.status_code = 404;
            responce.status_text = "Not Found";
            responce.body = "404 Not Found";
            responce.headers["Content-Type"] = "text/plain";
        }

        // Отправка ответа
        std::string out = responce.serialize();

        printResponce(out);
        std::cout << "method: " << request.method << std::endl << "==========="<< std::endl;

        send(client_fd, out.c_str(), out.size(), 0);
        close(client_fd);
    }
}

void HTTPServer::printResponce(const std::string &out) const {
    std::cout << "=== HTTP Response ===\n";
    const std::string::size_type header_end = out.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::string str = out.substr(0, header_end + 4);
        std::string line;
        std::string result;
        std::istringstream stream(str);
        while (std::getline(stream, line)) {
            if (!line.empty()) {
                result += line + "\n";
            }
        }
        std::cout << result;
    }
    else {
        std::cout << out;
    }
}

std::string HTTPServer::loadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// addRoute
void HTTPServer::addRoute(const std::string& method, const std::string& path, Handler handler) {
    routes[{method,path}] = std::move(handler);
}

void HTTPServer::addStaticRoute(const std::string& uri, const std::string& filePath, const std::string& contentType) {
    addRoute("GET", uri,
        /*lambda handler:*/ [filePath, contentType](const HTTPRequest&){
            HTTPResponse res;
            res.headers["Content-Type"] = contentType;
            res.body = loadFile(filePath);
            return res;
        });
}

void HTTPServer::addPostRoute(const std::string& path, Handler h) {
    addRoute("POST", path, std::move(h));
}

void HTTPServer::addStaticPostRoute() {
    addPostRoute("/echo", [](const HTTPRequest& req) {
        HTTPResponse res;
        res.headers["Content-Type"] = "text/plain";
        res.body = req.body;
        return res;
    });
}

void HTTPServer::addPutRoute(const std::string& path, Handler h) {
    addRoute("PUT", path, std::move(h));
}

void HTTPServer::addStaticPutRoute() {
    addPutRoute("/echo", [](const HTTPRequest& req) {
        HTTPResponse res;
        res.headers["Content-Type"] = "text/plain";
        res.body = "updated:" + req.body;
        return res;
    });
}

// void HTTPServer::addEchoRoutes() {
//         std::string resourceBody;
//     addPostRoute("/echo", [this, resourceBody](auto& req){
//         resourceBody = req.body;
//         HTTPResponse r; r.headers["Content-Type"]="text/plain"; r.body=resourceBody; return r;
//     });

//     addPutRoute("/echo", [this, resourceBody](auto& req){
//         resourceBody = "[updated] "+req.body;
//         HTTPResponse r; r.headers["Content-Type"]="text/plain"; r.body=resourceBody; return r;
//     });

//     addRoute("GET","/echo",[this](auto&){
//         HTTPResponse r; r.headers["Content-Type"]="text/plain"; r.body=resourceBody; return r;
//     });
// }
