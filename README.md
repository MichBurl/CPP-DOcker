# Distributed Banking System C++

## Scope
This project is a simulation of a distributed Banking System implemented in C++17. It utilizes a custom TCP/IP protocol to demonstrate multi-threaded server architecture, thread-safe concurrency (Mutex/Scoped Locks), and secure communication (XOR Encryption) in a containerized environment.

The architecture consists of:

* **BankServer**: A multi-threaded C++ TCP server that manages accounts, processes transactions, and persists data to a local file using a Thread Pool.
* **ATM Client**: A console application that simulates user behavior (Deposit, Withdraw, Transfer, History) with randomized logic.
* **AdminPanel**: A monitoring service implementing the Publisher-Subscriber pattern to display encrypted transaction logs in real-time.
* **Docker Compose**: Used for network orchestration and volume mapping for data persistence.

## Prerequisites
* Docker & Docker Compose

## How to run the project

### Reset Data (Optional)
To ensure the server generates a fresh set of 20 accounts (ID 100-119), clear the existing database file before starting:

```bash
rm bank_data.txt
touch bank_data.txt
```
Start the environment
Build and run the server, admin panel, and all 10 ATM instances:
```bash
docker-compose up --build
```
Monitor Transactions (Admin Panel)
The system uses a dedicated Admin Panel to decrypt and visualize traffic. To see the colored transaction logs in real-time:
```bash
docker logs -f admin-panel
```
Output Example:
```bash
[WPLATA]     [LOG] Wplata: 100 na konto 115
[TRANSFER]   [LOG] TRANSFER: 25 | 112 -> 115
[WYPLATA]    [LOG] Wyplata: 50 z konta 103
```
Verify Individual ATM History
To verify the transaction history and local actions of a specific ATM (e.g., ATM #4):
```bash
docker logs -f atm-4
```
## Features & Architecture

### Concurrency & Scaling

The system is pre-configured with 10 concurrent ATM clients attacking the server simultaneously. The Server uses a fixed Thread Pool (8 threads) to handle these requests, ensuring resource optimization and preventing thread explosion under high load.

### Persistence

Data is decoupled from the container lifecycle. All account balances and transaction histories are saved to bank_data.txt on the host machine via Docker Volumes.

### Security

All network traffic between ATMs, the Server, and the Admin Panel is encrypted using XOR Cipher logic to prevent plaintext sniffing within the internal network.
