#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../common/Protocol.h"
#include "Account.h"

// Baza danych kont (w pamięci)
std::map<int, Account*> accounts;

// Funkcja obsługująca jednego klienta (uruchamiana w osobnym wątku)
void handleClient(int clientSocket) {
    Message msg;
    while (true) {
        // 1. Odbierz dane od klienta
        int bytesRead = read(clientSocket, &msg, sizeof(msg));
        if (bytesRead <= 0) break; // Klient się rozłączył

        Response res;
        res.success = false;
        
        // 2. Znajdź konto
        if (accounts.find(msg.account_id) != accounts.end()) {
            Account* acc = accounts[msg.account_id];
            
            // 3. Wykonaj akcję
            if (msg.action == DEPOSIT) {
                acc->deposit(msg.amount);
                res.success = true;
                strcpy(res.message, "Wplata OK");
            } 
            else if (msg.action == WITHDRAW) {
                res.success = acc->withdraw(msg.amount);
                strcpy(res.message, res.success ? "Wyplata OK" : "Brak srodkow");
            }
            else if (msg.action == BALANCE) {
                res.success = true;
                strcpy(res.message, "Stan konta");
            }
            res.current_balance = acc->getBalance();
        } else {
            strcpy(res.message, "Nieznane konto");
        }

        // 4. Odeślij odpowiedź
        write(clientSocket, &res, sizeof(res));
    }
    close(clientSocket);
}

int main() {
    // Inicjalizacja przykładowych kont
    accounts[100] = new Account(100, 1000.0); // Konto Jana
    accounts[101] = new Account(101, 5000.0); // Konto Anny

    // Tworzenie gniazda (socket)
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Port 8080
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Bank Server dziala na porcie 8080..." << std::endl;

    while (true) {
        // Czekaj na klienta
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::cout << "Nowy klient polaczony!" << std::endl;

        // WSPÓŁBIEŻNOŚĆ: Uruchom nowy wątek dla tego klienta
        // Dzięki detach() wątek działa w tle, a pętla główna wraca czekać na kolejnych
        std::thread(handleClient, clientSocket).detach();
    }
    return 0;
}