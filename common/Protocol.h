#ifndef PROTOCOL_H
#define PROTOCOL_H

// Typy operacji
enum ActionType {
    BALANCE = 1,
    DEPOSIT = 2,
    WITHDRAW = 3
};

// Struktura przesyłana przez sieć (prosta serializacja binarna)
struct Message {
    int account_id;
    ActionType action;
    double amount;
};

// Struktura odpowiedzi
struct Response {
    bool success;
    double current_balance;
    char message[64]; // Krótki komunikat tekstowy
};

#endif