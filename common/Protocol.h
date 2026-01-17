#ifndef PROTOCOL_H
#define PROTOCOL_H

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

#endif