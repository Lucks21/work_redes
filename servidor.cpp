/*
 * servidor.cpp
 * 
 * g++ -std=c++11 servidor.cpp -o servidor -lpthread
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <cstdlib>
#include <ctime>

using namespace std;

std::mutex mtx;

class CuatroEnLinea {
public:
    CuatroEnLinea();
    void mostrarTablero();
    bool hacerJugada(int columna);
    bool comprobarGanador();
    bool comprobarEmpate();
    char obtenerTurno() const;
    std::string obtenerTableroComoString() const;

private:
    std::vector<std::vector<char>> tablero;
    char turno;

    bool comprobarFilas();
    bool comprobarColumnas();
    bool comprobarDiagonales();
};

// Implementación de los métodos de la clase CuatroEnLinea
CuatroEnLinea::CuatroEnLinea() : tablero(6, std::vector<char>(7, ' ')), turno('C') {}

void CuatroEnLinea::mostrarTablero() {
    for (const auto& fila : tablero) {
        for (const auto& celda : fila) {
            std::cout << "|" << celda;
        }
        std::cout << "|\n";
    }
    std::cout << "---------------\n";
    std::cout << " 1 2 3 4 5 6 7\n";
}

bool CuatroEnLinea::hacerJugada(int columna) {
    if (columna < 1 || columna > 7 || tablero[0][columna-1] != ' ') {
        std::cout << "Columna no válida. Inténtelo de nuevo.\n";
        return false;
    }

    for (int fila = 5; fila >= 0; --fila) {
        if (tablero[fila][columna-1] == ' ') {
            tablero[fila][columna-1] = turno;
            turno = (turno == 'C') ? 'S' : 'C';
            return true;
        }
    }
    return false;
}

bool CuatroEnLinea::comprobarGanador() {
    return comprobarFilas() || comprobarColumnas() || comprobarDiagonales();
}

bool CuatroEnLinea::comprobarEmpate() {
    for (const auto& fila : tablero) {
        for (const auto& celda : fila) {
            if (celda == ' ') {
                return false;
            }
        }
    }
    return true;
}

char CuatroEnLinea::obtenerTurno() const {
    return turno;
}

std::string CuatroEnLinea::obtenerTableroComoString() const {
    std::string tableroString;
    for (const auto& fila : tablero) {
        for (const auto& celda : fila) {
            tableroString += "|";
            tableroString += celda;
        }
        tableroString += "|\n";
    }
    tableroString += "---------------\n";
    tableroString += " 1 2 3 4 5 6 7\n";
    return tableroString;
}

bool CuatroEnLinea::comprobarFilas() {
    for (int fila = 0; fila < 6; ++fila) {
        for (int col = 0; col < 4; ++col) {
            if (tablero[fila][col] != ' ' && 
                tablero[fila][col] == tablero[fila][col+1] && 
                tablero[fila][col] == tablero[fila][col+2] && 
                tablero[fila][col] == tablero[fila][col+3]) {
                return true;
            }
        }
    }
    return false;
}

bool CuatroEnLinea::comprobarColumnas() {
    for (int col = 0; col < 7; ++col) {
        for (int fila = 0; fila < 3; ++fila) {
            if (tablero[fila][col] != ' ' && 
                tablero[fila][col] == tablero[fila+1][col] && 
                tablero[fila][col] == tablero[fila+2][col] && 
                tablero[fila][col] == tablero[fila+3][col]) {
                return true;
            }
        }
    }
    return false;
}

bool CuatroEnLinea::comprobarDiagonales() {
    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 4; ++col) {
            if (tablero[fila][col] != ' ' && 
                tablero[fila][col] == tablero[fila+1][col+1] && 
                tablero[fila][col] == tablero[fila+2][col+2] && 
                tablero[fila][col] == tablero[fila+3][col+3]) {
                return true;
            }
        }
        for (int col = 3; col < 7; ++col) {
            if (tablero[fila][col] != ' ' && 
                tablero[fila][col] == tablero[fila+1][col-1] && 
                tablero[fila][col] == tablero[fila+2][col-2] && 
                tablero[fila][col] == tablero[fila+3][col-3]) {
                return true;
            }
        }
    }
    return false;
}

void enviarMensaje(int clienteSocket, const std::string& mensaje) {
    send(clienteSocket, mensaje.c_str(), mensaje.length(), 0);
}

void manejarCliente(int socket_cliente, struct sockaddr_in direccionCliente) {
    CuatroEnLinea juego;
    srand(time(0));
    bool esTurnoCliente = rand() % 2 == 0;
    enviarMensaje(socket_cliente, esTurnoCliente ? "Tu turno\n" : "Turno del servidor\n");

    char buffer[1024];
    bool ganador = false;
    bool empate = false;
    int columna;

    while (!ganador && !empate) {
        if (esTurnoCliente) {
            memset(buffer, 0, sizeof(buffer));
            int valread = recv(socket_cliente, buffer, 1024, 0);
            if (valread > 0) {
                columna = std::stoi(buffer);
                if (juego.hacerJugada(columna)) {
                    ganador = juego.comprobarGanador();
                    empate = juego.comprobarEmpate();
                    esTurnoCliente = false;
                    std::string tableroString = juego.obtenerTableroComoString();
                    enviarMensaje(socket_cliente, tableroString);
                } else {
                    enviarMensaje(socket_cliente, "Columna no válida. Inténtelo de nuevo.\n");
                }
            }
        } else {
            // Lógica del turno del servidor
            do {
                columna = rand() % 7 + 1; // Generar columna aleatoria
            } while (!juego.hacerJugada(columna));
            ganador = juego.comprobarGanador();
            empate = juego.comprobarEmpate();
            esTurnoCliente = true;
            std::string tableroString = juego.obtenerTableroComoString();
            enviarMensaje(socket_cliente, tableroString);
            enviarMensaje(socket_cliente, "Tu turno\n");
        }

        if (ganador) {
            enviarMensaje(socket_cliente, "Fin del juego, alguien ha ganado\n");
        } else if (empate) {
            enviarMensaje(socket_cliente, "Fin del juego, empate\n");
        }
    }

    close(socket_cliente);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Uso: ./servidor <puerto>" << endl;
        return -1;
    }

    int port = atoi(argv[1]);
    int socket_server = 0;
    struct sockaddr_in direccionServidor, direccionCliente;

    cout << "Creando socket de escucha...\n";
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Error al crear el socket de escucha\n";
        exit(EXIT_FAILURE);
    }

    cout << "Configurando estructura de dirección del socket...\n";
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(port);

    cout << "Vinculando socket...\n";
    if (bind(socket_server, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
        cout << "Error en bind()\n";
        exit(EXIT_FAILURE);
    }

    cout << "Llamando a listen...\n";
    if (listen(socket_server, 1024) < 0) {
        cout << "Error en listen()\n";
        exit(EXIT_FAILURE);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    cout << "Esperando solicitud de cliente...\n";

    vector<thread> hilos;

    while (true) {
        int socket_cliente;
        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size)) < 0) {
            cout << "Error en accept()\n";
            exit(EXIT_FAILURE);
        }

        hilos.emplace_back(manejarCliente, socket_cliente, direccionCliente);
    }

    for (auto &hilo : hilos) {
        hilo.join();
    }

    return 0;
}