#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <mutex>
#include <iostream>

// OOP: Klasa reprezentująca pojedyncze konto
class Account {
private:
    int id;
    double balance;
    mutable std::mutex mtx; // WSPÓŁBIEŻNOŚĆ: Strażnik dostępu do tego konta

public:
    Account(int id, double initial_balance) : id(id), balance(initial_balance) {}

    // Metoda do wpłaty (bezpieczna wątkowo)
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx); // Zablokuj dostęp dla innych wątków
        balance += amount;
        std::cout << "[Konto " << id << "] Wplacono: " << amount << ". Nowe saldo: " << balance << std::endl;
    }

    // Metoda do wypłaty (zwraca true jeśli się udało)
    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx); // Zablokuj
        if (balance >= amount) {
            balance -= amount;
            std::cout << "[Konto " << id << "] Wyplacono: " << amount << ". Nowe saldo: " << balance << std::endl;
            return true;
        } else {
            std::cout << "[Konto " << id << "] Odmowa wyplaty " << amount << ". Brak srodkow." << std::endl;
            return false;
        }
    }

    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx); // Nawet odczyt warto zabezpieczyć
        return balance;
    }

    int getId() const { return id; }
};

#endif