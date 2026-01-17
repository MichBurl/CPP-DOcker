#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <iomanip>
#include "../common/Protocol.h"

// --- KODY KOLORÓW ANSI ---
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";     // Wyplata / Blad
const std::string GREEN   = "\033[32m";     // Wplata
const std::string YELLOW  = "\033[33m";     // Transfer
const std::string CYAN    = "\033[36m";     // Info
const std::string BOLD    = "\033[1m";

std::string getHostIp(const std::string& hostname) {
    struct hostent* he = gethostbyname(hostname.c_str());
    if (he == nullptr) return "127.0.0.1";
    return inet_ntoa(*(struct in_addr*)he->h_addr);
}

// Funkcja pomocnicza do kolorowania logów na podstawie treści
void printColoredLog(const std::string& log) {
    std::string color = RESET;
    std::string prefix = "[ INFO ]";

    if (log.find("Wplata") != std::string::npos) {
        color = GREEN;
        prefix = "[WPLATA]";
    } else if (log.find("Wyplata") != std::string::npos) {
        color = RED;
        prefix = "[WYPLATA]";
    } else if (log.find("TRANSFER") != std::string::npos) {
        color = YELLOW;
        prefix = "[TRANSFER]";
    } else if (log.find("Blad") != std::string::npos) {
        color = RED + BOLD;
        prefix = "[!BLAD!]";
    }

    std::cout << color << std::left << std::setw(12) << prefix 
              << " " << log << RESET << std::endl;
}

int main() {
    std::string serverIp = getHostIp("bank-server");
    std::cout << BOLD << ">>> ADMIN PANEL connecting to " << serverIp << "..." << RESET << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, serverIp.c_str(), &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << RED << "Connection failed" << RESET << std::endl;
        return -1;
    }

    // Logowanie
    Message loginMsg;
    loginMsg.action = ADMIN_LOGIN;
    cipher(&loginMsg, sizeof(loginMsg));
    write(sock, &loginMsg, sizeof(loginMsg));
    
    std::cout << CYAN << ">>> Zalogowano. Monitoring aktywny." << RESET << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    while (true) {
        int len;
        int bytes = read(sock, &len, sizeof(len));
        if (bytes <= 0) break;

        char* buffer = new char[len + 1];
        read(sock, buffer, len);
        buffer[len] = '\0';

        cipher(buffer, len); // Odszyfruj

        printColoredLog(std::string(buffer));
        
        delete[] buffer;
    }

    close(sock);
    return 0;
}