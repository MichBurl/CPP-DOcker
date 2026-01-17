#ifndef BANK_H
#define BANK_H

#include <map>
#include <mutex>
#include <fstream>
#include <iostream>
#include <filesystem> // C++17
#include "Account.h"

class Bank {
private:
    std::map<int, Account*> accounts;
    std::mutex bankMtx;
    const std::string dbFile = "bank_data.txt";

    void save() {
        std::lock_guard<std::mutex> lock(bankMtx); 
        std::ofstream file(dbFile);
        if (file.is_open()) {
            for (const auto& pair : accounts) {
                file << pair.first << " " << pair.second->getBalance() << "\n";
            }
        }
    }

    void load() {
        std::ifstream file(dbFile);
        if (file.is_open()) {
            int id;
            double balance;
            while (file >> id >> balance) {
                accounts[id] = new Account(id, balance);
            }
            std::cout << ">>> Wczytano dane z pliku." << std::endl;
        }
    }

public:
    Bank() {
        load();
        if (accounts.empty()) {
            addAccount(100, 1000.0);
            addAccount(101, 1000.0);
            addAccount(102, 1000.0);
            save();
        }
    }

    ~Bank() {
        save();
        for (auto& pair : accounts) delete pair.second;
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
            return true;
        }
        return false;
    }

    bool withdraw(int id, double amount) {
        Account* acc = getAccount(id);
        if (acc && acc->withdraw(amount)) {
            save(); 
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
                success = true;
                std::cout << "Transfer: " << amount << " (" << fromId << "->" << toId << ")" << std::endl;
            }
        }
        if (success) {
            save();
        }

        return success;
    }
};

#endif