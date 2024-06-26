/*
 * cliente.cpp
 * 
 * g++ -std=c++11 cliente.cpp -o cliente
 *
 */

#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <atomic>

using namespace std;

atomic<bool> juegoTerminado(false);

void recibirMensajes(int socket_cliente) {
    char buffer[1024];
    while (!juegoTerminado) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(socket_cliente, buffer, 1024);
        if (valread > 0) {
            cout << buffer << endl;
            if (strstr(buffer, "Fin del juego")) {
                cout << "El juego ha terminado.\n";
                juegoTerminado = true;
                close(socket_cliente);
                return;
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        cout << "Uso: ./cliente <direccion_ip> <puerto>" << endl;
        return -1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char mensaje[1024];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "\n Error al crear el socket \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        cout << "\n Dirección no válida o no soportada \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "\n Fallo en la conexión \n";
        return -1;
    }

    thread recibir(recibirMensajes, sock);

    while (!juegoTerminado) {
        cout << "\nIntroduzca la columna (1-7): ";
        cin >> mensaje;
        if (juegoTerminado) {
            break;
        }
        send(sock, mensaje, strlen(mensaje), 0);
    }

    recibir.join();
    close(sock);
    return 0;
}