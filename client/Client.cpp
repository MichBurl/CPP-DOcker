#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "../common/Protocol.h"

class ATM {
private:
    int id;
    int socketFd;
    bool isConnected;

    std::string getHostIp(const std::string& hostname) {
        struct hostent* he = gethostbyname(hostname.c_str());
        if (he == nullptr) return "127.0.0.1";
        return inet_ntoa(*(struct in_addr*)he->h_addr);
    }

public:
    ATM(int atmId) : id(atmId), socketFd(-1), isConnected(false) {}

    bool connectToBank(const std::string& hostname, int port) {
        std::string ip = getHostIp(hostname);
        std::cout << "[ATM " << id << "] Laczenie z " << ip << "..." << std::endl;

        socketFd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);

        if (connect(socketFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "[ATM " << id << "] Blad polaczenia!" << std::endl;
            return false;
        }
        isConnected = true;
        return true;
    }

    void processRandomTransactions(int count) {
        if (!isConnected) return;
        
        srand(time(NULL) + id);

        for (int i = 0; i < count; ++i) {
            performSingleTransaction();
            // Symulacja czasu reakcji użytkownika
            std::this_thread::sleep_for(std::chrono::milliseconds(500 + (rand() % 1500)));
        }
    }

private:
    void performSingleTransaction() {
Message msg;
        int numAccounts = 20;
        msg.account_id = 100 + (rand() % numAccounts); 
        
        // Zwiększamy zakres losowania do 5 (0,1,2,3,4)
        int action = rand() % 5; 

        if (action == 0) { msg.action = DEPOSIT; msg.amount = 100.0; }
        else if (action == 1) { msg.action = WITHDRAW; msg.amount = 50.0; }
        else if (action == 2) { msg.action = BALANCE; msg.amount = 0; }
        else if (action == 3) {
            msg.action = TRANSFER; msg.amount = 25.0;
            do { msg.target_account_id = 100 + (rand() % numAccounts); } 
            while (msg.target_account_id == msg.account_id);
        } else {
            msg.action = GET_HISTORY;
            msg.amount = 0;
        }

        Message encryptedMsg = msg; 
        cipher(&encryptedMsg, sizeof(encryptedMsg));
        write(socketFd, &encryptedMsg, sizeof(encryptedMsg));
        
        if (msg.action == GET_HISTORY) {
            Response header;
            read(socketFd, &header, sizeof(header));
            cipher(&header, sizeof(header));

            if (header.success) {
                int count = (int)header.current_balance;
                std::cout << "[ATM " << id << "] --- HISTORIA KONTA " << msg.account_id << " (" << count << ") ---" << std::endl;
                
                for (int i = 0; i < count; ++i) {
                    Response entry;
                    read(socketFd, &entry, sizeof(entry));
                    cipher(&entry, sizeof(entry));
                    std::cout << "    " << i+1 << ". " << entry.message << std::endl;
                }
                std::cout << "-----------------------------------" << std::endl;
            } else {
                 std::cout << "[ATM " << id << "] Blad pobierania historii: " << header.message << std::endl;
            }
            return;
        }

        Response res;
        read(socketFd, &res, sizeof(res));
        cipher(&res, sizeof(res));

        std::cout << "[ATM " << id << "] Konto " << msg.account_id 
                  << (msg.action == TRANSFER ? " -> TRANSFER do " + std::to_string(msg.target_account_id) : " AKCJA")
                  << " | " << res.message 
                  << " | Saldo: " << res.current_balance << std::endl;
    }

public:
    ~ATM() {
        if (isConnected) close(socketFd);
    }
};

int main() {
    const char* idEnv = std::getenv("CLIENT_ID");
    int id = idEnv ? atoi(idEnv) : 1;

    ATM myAtm(id);
    
    if (myAtm.connectToBank("bank-server", 8080)) {
        myAtm.processRandomTransactions(10);
    } else {
        std::cout << "[ATM " << id << "] Awaria systemu." << std::endl;
    }

    return 0;
}