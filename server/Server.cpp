#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../common/Protocol.h"
#include "Bank.h"

Bank myBank;

void handleClient(int clientSocket) {
    Message msg;
    while (true) {
        int bytesRead = read(clientSocket, &msg, sizeof(msg));
        if (bytesRead <= 0) break;

        Response res;
        res.success = false;
        
        Account* acc = myBank.getAccount(msg.account_id);

        if (acc) {
            if (msg.action == DEPOSIT) {
                acc->deposit(msg.amount);
                res.success = true;
                strcpy(res.message, "Wplata OK");
            } 
            else if (msg.action == WITHDRAW) {
                res.success = acc->withdraw(msg.amount);
                strcpy(res.message, res.success ? "Wyplata OK" : "Brak srodkow");
            }
            else if (msg.action == TRANSFER) {
                bool ok = myBank.transfer(msg.account_id, msg.target_account_id, msg.amount);
                res.success = ok;
                strcpy(res.message, ok ? "Przelew wyslany" : "Blad przelewu");
            }
            else if (msg.action == BALANCE) {
                res.success = true;
                strcpy(res.message, "Stan konta");
            }
            res.current_balance = acc->getBalance();
        } else {
            strcpy(res.message, "Nieznane konto");
            res.current_balance = 0;
        }

        write(clientSocket, &res, sizeof(res));
    }
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 10); // Zwiększamy kolejkę dla 4 bankomatów

    std::cout << ">>> BANK SERVER READY (OOP Version) <<<" << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::thread(handleClient, clientSocket).detach();
    }
    return 0;
}