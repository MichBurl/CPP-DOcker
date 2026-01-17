#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../common/Protocol.h"
#include "Bank.h"
#include "ThreadPool.h"

Bank myBank;

void handleClient(int clientSocket) {
    Message msg;
    while (true) {
        if (read(clientSocket, &msg, sizeof(msg)) <= 0) break;

        Response res{false, 0.0, ""};
        Account* acc = myBank.getAccount(msg.account_id);

        if (acc) {
            // Obsługa logiki (taka sama jak w kroku 1)
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
            else if (msg.action == TRANSFER) {
                res.success = myBank.transfer(msg.account_id, msg.target_account_id, msg.amount);
                strcpy(res.message, res.success ? "Przelew OK" : "Blad przelewu");
            }
            res.current_balance = acc->getBalance();
        } else {
            strcpy(res.message, "Nieznane konto");
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
    listen(serverSocket, 100);

    std::cout << ">>> SERVER READY (Pool Version) <<<" << std::endl;
    
    // Tworzymy pulę 4 stałych wątków
    ThreadPool pool(4); 

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        
        // Zamiast tworzyć nowy wątek, wrzucamy zadanie do kolejki
        pool.enqueue([clientSocket] {
            handleClient(clientSocket);
        });
    }
    return 0;
}