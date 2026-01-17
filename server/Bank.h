#ifndef BANK_H
#define BANK_H

#include <map>
#include <vector>
#include <mutex>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include "Account.h"
#include "../common/Protocol.h"

class Bank {
private:
    std::map<int, Account*> accounts;
    std::mutex bankMtx;
    
    std::vector<int> adminSockets;
    std::mutex adminMtx;
    
    const std::string dbFile = "bank_data.txt";

    void save() {
        std::lock_guard<std::mutex> lock(bankMtx); 
        std::ofstream file(dbFile);
        if (file.is_open()) {
            for (const auto& pair : accounts) {
                Account* acc = pair.second;
                auto hist = acc->getHistory(); 
                
                // Format: ID SALDO LICZBA_WPISÓW
                file << pair.first << " " << acc->getBalance() << " " << hist.size() << "\n";
                
                for (const auto& entry : hist) {
                    file << entry << "\n";
                }
            }
        }
    }

    void load() {
        std::ifstream file(dbFile);
        if (file.is_open()) {
            int id;
            double balance;
            int historyCount;

            while (file >> id >> balance >> historyCount) {
                accounts[id] = new Account(id, balance);
                
                accounts[id]->clearHistory();

                std::string dummy;
                std::getline(file, dummy); 

                for (int i = 0; i < historyCount; ++i) {
                    std::string line;
                    if (std::getline(file, line)) {
                        accounts[id]->addHistory(line);
                    }
                }
            }
            std::cout << ">>> Wczytano dane i historie z pliku." << std::endl;
        }
    }

    void broadcast(const std::string& msg) {
        std::lock_guard<std::mutex> lock(adminMtx);
        
        for (auto it = adminSockets.begin(); it != adminSockets.end(); ) {
            int len = msg.length();
            
            std::string encryptedMsg = msg;
            cipher((void*)encryptedMsg.data(), len);

            int w1 = write(*it, &len, sizeof(len));
            int w2 = write(*it, encryptedMsg.data(), len);
            
            if (w1 <= 0 || w2 <= 0) {
                close(*it);
                it = adminSockets.erase(it);
            } else {
                ++it;
            }
        }
    }

public:
    Bank() {
        load();
        if (accounts.empty()) {
            std::cout << ">>> Generowanie 20 nowych kont..." << std::endl;
            for (int i = 0; i < 20; ++i) {
                addAccount(100 + i, 1000.0);
            }
            save();
        }
    }

    ~Bank() {
        save();
        for (auto& pair : accounts) delete pair.second;
    }

    void addAdmin(int socket) {
        std::lock_guard<std::mutex> lock(adminMtx);
        adminSockets.push_back(socket);
        std::cout << ">>> ADMIN LOGGED IN (Socket: " << socket << ")" << std::endl;
    }

    void addAccount(int id, double balance) {
        std::lock_guard<std::mutex> lock(bankMtx);
        if (accounts.find(id) == accounts.end()) {
            accounts[id] = new Account(id, balance);
        }
    }

    Account* getAccount(int id) {
        std::lock_guard<std::mutex> lock(bankMtx);
        if (accounts.find(id) != accounts.end()) return accounts[id];
        return nullptr;
    }

    bool deposit(int id, double amount) {
        Account* acc = getAccount(id);
        if (acc) {
            acc->deposit(amount);
            save();
            broadcast("[LOG] Wplata: " + std::to_string((int)amount) + " na konto " + std::to_string(id));
            return true;
        }
        return false;
    }

    bool withdraw(int id, double amount) {
        Account* acc = getAccount(id);
        if (acc && acc->withdraw(amount)) {
            save();
            broadcast("[LOG] Wyplata: " + std::to_string((int)amount) + " z konta " + std::to_string(id));
            return true;
        }
        return false;
    }

    bool transfer(int fromId, int toId, double amount) {
        Account* fromAcc = getAccount(fromId);
        Account* toAcc = getAccount(toId);

        if (!fromAcc || !toAcc || fromId == toId) return false;

        bool success = false;
        {
            std::scoped_lock lock(fromAcc->mtx, toAcc->mtx);
            
            if (fromAcc->balance >= amount) {
                fromAcc->balance -= amount;
                toAcc->balance += amount;
                
                fromAcc->addHistory("Przelew do " + std::to_string(toId) + ": -" + std::to_string((int)amount));
                toAcc->addHistory("Przelew od " + std::to_string(fromId) + ": +" + std::to_string((int)amount));
                
                success = true;
            }
        } 

        if (success) {
            save();
            broadcast("[LOG] TRANSFER: " + std::to_string((int)amount) + " | " + std::to_string(fromId) + " -> " + std::to_string(toId));
            return true;
        }
        return false;
    }
};

#endif