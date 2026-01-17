FROM gcc:latest

WORKDIR /app
COPY . .

# Flaga c++17 dla wszystkich
RUN g++ -std=c++17 -o server_app server/Server.cpp -pthread
RUN g++ -std=c++17 -o client_app client/Client.cpp -pthread
# NOWOŚĆ: Kompilacja admina
RUN g++ -std=c++17 -o admin_app client/Admin.cpp -pthread

CMD ["bash"]