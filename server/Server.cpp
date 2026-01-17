#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include "../common/Protocol.h"
#include "Bank.h"
#include "ThreadPool.h"

Bank myBank;

void handleClient(int clientSocket) {
    Message msg;

    int bytesRead = read(clientSocket, &msg, sizeof(msg));
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }

    cipher(&msg, sizeof(msg));

    // --- SCIEŻKA ADMINA ---
    if (msg.action == ADMIN_LOGIN) {
        myBank.addAdmin(clientSocket);
        
        char dummy;
        while (read(clientSocket, &dummy, 1) > 0) {
        }
        std::cout << ">>> Admin rozlaczony." << std::endl;
        close(clientSocket);
        return;
    }

    // --- SCIEŻKA BANKOMATU ---
    do {
        Response res{false, 0.0, ""};
        
        if (msg.action == GET_HISTORY) {
            Account* acc = myBank.getAccount(msg.account_id);
            if (acc) {
                // Pobieramy historię z konta
                std::vector<std::string> history = acc->getHistory();
                
                res.success = true;
                res.current_balance = history.size();
                strcpy(res.message, "START_HISTORY");
                
                cipher(&res, sizeof(res));
                write(clientSocket, &res, sizeof(res));

                for (const auto& entry : history) {
                    Response hRes;
                    hRes.success = true;
                    hRes.current_balance = 0;
                    strncpy(hRes.message, entry.c_str(), 63);
                    hRes.message[63] = '\0';

                    cipher(&hRes, sizeof(hRes));
                    write(clientSocket, &hRes, sizeof(hRes));
                }

                bytesRead = read(clientSocket, &msg, sizeof(msg));
                if (bytesRead > 0) cipher(&msg, sizeof(msg));
                continue; 

            } else {
                strcpy(res.message, "Nieznane konto");
                res.success = false;
            }
        }

        if (msg.action == DEPOSIT) {
            res.success = myBank.deposit(msg.account_id, msg.amount);
            strcpy(res.message, res.success ? "Wplata OK" : "Blad");
        } 
        else if (msg.action == WITHDRAW) {
            res.success = myBank.withdraw(msg.account_id, msg.amount);
            strcpy(res.message, res.success ? "Wyplata OK" : "Brak srodkow");
        }
        else if (msg.action == TRANSFER) {
            res.success = myBank.transfer(msg.account_id, msg.target_account_id, msg.amount);
            strcpy(res.message, res.success ? "Przelew OK" : "Blad");
        }
        else if (msg.action == BALANCE) {
            res.success = true;
            strcpy(res.message, "Stan konta");
        }

        Account* acc = myBank.getAccount(msg.account_id);
        res.current_balance = acc ? acc->getBalance() : 0;
        if (!acc && msg.action != GET_HISTORY) strcpy(res.message, "Nieznane konto");

        cipher(&res, sizeof(res));
        write(clientSocket, &res, sizeof(res));

        bytesRead = read(clientSocket, &msg, sizeof(msg));
        if (bytesRead > 0) {
            cipher(&msg, sizeof(msg));
        }

    } while (bytesRead > 0);

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

    std::cout << ">>> SERVER READY (History + Secure + Admin) <<<" << std::endl;
    
    ThreadPool pool(8); 

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        pool.enqueue([clientSocket] {
            handleClient(clientSocket);
        });
    }
    return 0;
}