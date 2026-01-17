#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>       // Do rozwiązywania nazw hostów (DNS)
#include <thread>        // Do sleep_for
#include <chrono>        // Do czasu
#include <cstdlib>       // Do rand()
#include "../common/Protocol.h"

// Funkcja pomocnicza do pobrania IP serwera z nazwy (np. "bank-server")
std::string getHostIp(const std::string& hostname) {
    struct hostent* he = gethostbyname(hostname.c_str());
    if (he == nullptr) return "";
    return inet_ntoa(*(struct in_addr*)he->h_addr);
}

void runAutomatedATM(int clientId) {
    // 1. Rozwiąż nazwę serwera (w Dockerze serwer będzie się nazywał "bank-server")
    std::string serverIp = getHostIp("bank-server");
    if (serverIp.empty()) {
        // Fallback dla testów lokalnych bez docker-compose
        serverIp = "127.0.0.1"; 
    }

    std::cout << "[Klient " << clientId << "] Laczenie z " << serverIp << "..." << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, serverIp.c_str(), &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "[Klient " << clientId << "] Blad polaczenia!" << std::endl;
        return;
    }

    // Pętla symulująca działanie klienta
    srand(time(NULL) + clientId); // Różne ziarno losowości dla każdego klienta
    int accountId = 100; // Wszyscy atakują konto Jana (ID 100)

    for (int i = 0; i < 5; ++i) { // Wykonaj 5 operacji i zakończ
        Message msg;
        msg.account_id = accountId;
        
        // Losuj akcję: 0 = Wpłata, 1 = Wypłata, 2 = Sprawdzenie
        int action = rand() % 3;
        if (action == 0) {
            msg.action = DEPOSIT;
            msg.amount = 100.0;
        } else if (action == 1) {
            msg.action = WITHDRAW;
            msg.amount = 50.0;
        } else {
            msg.action = BALANCE;
            msg.amount = 0;
        }

        // Wyślij
        write(sock, &msg, sizeof(msg));

        // Odbierz
        Response res;
        read(sock, &res, sizeof(res));

        std::cout << "[ATM-" << clientId << "] Operacja: " << msg.action 
                  << " | Wynik: " << res.message 
                  << " | Saldo: " << res.current_balance << std::endl;

        // Odczekaj chwilę (symulacja człowieka przy bankomacie)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 + (rand() % 2000)));
    }

    close(sock);
}

int main() {
    // Pobieramy ID klienta ze zmiennej środowiskowej (ustawimy to w docker-compose)
    const char* idEnv = std::getenv("CLIENT_ID");
    int id = idEnv ? atoi(idEnv) : 1;
    
    runAutomatedATM(id);
    return 0;
}