FROM gcc:latest

WORKDIR /app

COPY . .

RUN g++ -std=c++17 -o server_app server/Server.cpp -pthread
RUN g++ -std=c++17 -o client_app client/Client.cpp -pthread

CMD ["bash"]