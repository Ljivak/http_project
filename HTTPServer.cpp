#include "HTTPServer.hpp"
#include <sstream>
#include <iostream>

HTTPRequest HTTPRequest::parse(int client_fd) {

    HTTPRequest req;
    std::string line;
    char buf[1024];
    std::stringstream ss;

    // Считать до разделителя заголовков "\r\n\r\n"
    size_t total = 0;
    while (true) {
        int n = recv(client_fd, buf, sizeof(buf), 0);
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
        auto pos = line.find(":");
        if (pos != std::string::npos) {
            std::string name = line.substr(0,pos);
            std::string value = line.substr(pos+1);
            // убрать \r
            if (!value.empty() && value.back() == '\r')
                value.pop_back();
            req.headers[name] = value;
        }
    }
    return req;
}

// HTTPResponse::serialize
std::string HTTPResponse::serialize() const {
    std::ostringstream out;
    out << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    for (auto& [k,v] : headers) {
        out << k << ": " << v << "\r\n";
    }
    out << "Content-Length: " << body.size() << "\r\n";
    out << "Connection: close\r\n";
    out << "\r\n";
    out << body;
    return out.str();
}

// Конструктор HTTPServer
HTTPServer::HTTPServer(int port, unsigned int interface)
    : server_socket(AF_INET, SOCK_STREAM, 0, port, interface)
{
    server_socket.establish_connection();
}

// addRoute
void HTTPServer::addRoute(const std::string& method, const std::string& path, Handler handler) {
    routes[{method,path}] = std::move(handler);
}

// listen
void HTTPServer::listen(int backlog) {
    server_socket.start_listening(backlog);
    std::cout << "HTTPServer listening..." << std::endl;

    while (true) {

        int client_fd = server_socket.accept_connection();
        // Парсинг запроса
        HTTPRequest req = HTTPRequest::parse(client_fd);

        // Поиск обработчика
        Handler h = nullptr;
        auto it = routes.find({req.method, req.path});
        if (it != routes.end()) h = it->second;

        HTTPResponse resp;
        if (h) {
            resp = h(req);
        } else {
            resp.status_code = 404;
            resp.status_text = "Not Found";
            resp.body = "404 Not Found";
            resp.headers["Content-Type"] = "text/plain";
        }

        // Отправка ответа
        std::string out = resp.serialize();
        send(client_fd, out.c_str(), out.size(), 0);
        close(client_fd);
    }
}