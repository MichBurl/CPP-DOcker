# Używamy oficjalnego obrazu GCC
FROM gcc:latest

# Ustawiamy katalog roboczy
WORKDIR /app

# Kopiujemy wszystkie pliki projektu do kontenera
COPY . .

# Kompilujemy Serwer (pamiętaj o fladze -pthread dla wątków!)
RUN g++ -o server_app server/Server.cpp -pthread

# Kompilujemy Klienta
RUN g++ -o client_app client/Client.cpp -pthread

# Domyślnie ten obraz nic nie robi, komendę startową podamy w docker-compose
CMD ["bash"]