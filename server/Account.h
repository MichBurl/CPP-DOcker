#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <mutex>
#include <vector>
#include <string>
#include <iostream>

class Account {
    friend class Bank; 

private:
    int id;
    double balance;
    std::vector<std::string> history;
    mutable std::mutex mtx; 

public:
    Account(int id, double initial_balance) : id(id), balance(initial_balance) {
        addHistory("Otwarcie konta: " + std::to_string((int)initial_balance));
    }

    void addHistory(const std::string& entry) {
        // Limitujemy historię do ostatnich 10 wpisów, żeby plik nie rósł w nieskończoność
        if (history.size() >= 10) {
            history.erase(history.begin());
        }
        history.push_back(entry);
    }
    
    void clearHistory() {
        history.clear();
    }

    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
        addHistory("Wplata: +" + std::to_string((int)amount));
    }

    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (balance >= amount) {
            balance -= amount;
            addHistory("Wyplata: -" + std::to_string((int)amount));
            return true;
        }
        return false;
    }

    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }

    std::vector<std::string> getHistory() const {
        std::lock_guard<std::mutex> lock(mtx);
        return history;
    }
};

#endif