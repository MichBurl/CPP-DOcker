#ifndef BANK_H
#define BANK_H

#include <map>
#include <mutex>
#include "Account.h"

class Bank {
private:
    std::map<int, Account*> accounts;
    std::mutex bankMtx;

public:
    Bank() {
        addAccount(100, 1000.0);
        addAccount(101, 1000.0);
        addAccount(102, 1000.0);
    }

    ~Bank() {
        for (auto& pair : accounts) {
            delete pair.second;
        }
    }

    void addAccount(int id, double balance) {
        std::lock_guard<std::mutex> lock(bankMtx);
        accounts[id] = new Account(id, balance);
    }

    Account* getAccount(int id) {
        std::lock_guard<std::mutex> lock(bankMtx);
        if (accounts.find(id) != accounts.end()) return accounts[id];
        return nullptr;
    }

    bool transfer(int fromId, int toId, double amount) {
        Account* fromAcc = getAccount(fromId);
        Account* toAcc = getAccount(toId);

        if (!fromAcc || !toAcc || fromId == toId) {
            std::cout << "Transfer failed: Invalid accounts" << std::endl;
            return false;
        }

        std::scoped_lock lock(fromAcc->mtx, toAcc->mtx);

        if (fromAcc->balance >= amount) {
            fromAcc->balance -= amount;
            toAcc->balance += amount;
            
            std::cout << ">>> TRANSFER SUKCES: " << amount 
                      << " z " << fromId << " do " << toId << std::endl;
            return true;
        } else {
            std::cout << ">>> TRANSFER BLAD: Brak srodkow u " << fromId << std::endl;
            return false;
        }
    }
};

#endif