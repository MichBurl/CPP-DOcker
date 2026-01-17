#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include "../common/Protocol.h"

std::string getHostIp(const std::string& hostname) {
    struct hostent* he = gethostbyname(hostname.c_str());
    if (he == nullptr) return "127.0.0.1";
    return inet_ntoa(*(struct in_addr*)he->h_addr);
}

int main() {
    std::string serverIp = getHostIp("bank-server");
    std::cout << ">>> ADMIN PANEL connecting to " << serverIp << "..." << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, serverIp.c_str(), &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    Message loginMsg;
    loginMsg.action = ADMIN_LOGIN;
    write(sock, &loginMsg, sizeof(loginMsg));
    std::cout << ">>> Zalogowano. Oczekiwanie na transakcje na zywo..." << std::endl;

    while (true) {
        int len;
        int bytes = read(sock, &len, sizeof(len));
        if (bytes <= 0) break;

        char* buffer = new char[len + 1];
        read(sock, buffer, len);
        buffer[len] = '\0';

        std::cout << buffer << std::endl;
        delete[] buffer;
    }

    close(sock);
    return 0;
}