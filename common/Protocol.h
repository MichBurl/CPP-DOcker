#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

// Klucz symetryczny (musi być taki sam u Klienta i Serwera)
const std::string SECRET_KEY = "SUPER_TAJNE_HASLO_BANKU_XYZ";

enum ActionType {
    BALANCE = 1,
    DEPOSIT = 2,
    WITHDRAW = 3,
    TRANSFER = 4,
    ADMIN_LOGIN = 99
};

struct Message {
    int account_id;
    int target_account_id;
    ActionType action;
    double amount;
};

struct Response {
    bool success;
    double current_balance;
    char message[64];
};

inline void cipher(void* data, size_t size) {
    char* bytes = (char*)data;
    for (size_t i = 0; i < size; ++i) {
        bytes[i] ^= SECRET_KEY[i % SECRET_KEY.length()];
    }
}

#endif