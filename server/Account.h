#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <mutex>
#include <iostream>

class Account {
    friend class Bank; 

private:
    int id;
    double balance;
    mutable std::mutex mtx; 

public:
    Account(int id, double initial_balance) : id(id), balance(initial_balance) {}

    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
    }

    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }

    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
};

#endif