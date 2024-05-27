#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char mensaje[1024];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Error al crear el socket \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7777);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "\n Dirección no válida o no soportada \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\n Fallo en la conexión \n";
        return -1;
    }

    std::thread leerMensajes([&]() {
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, 1024);
            if (valread > 0) {
                std::cout << buffer << std::endl;
            }
        }
    });

    while (true) {
        std::cout << "Introduzca la columna (1-7): ";
        std::cin >> mensaje;
        send(sock, mensaje, strlen(mensaje), 0);
    }

    leerMensajes.join();
    close(sock);
    return 0;
}