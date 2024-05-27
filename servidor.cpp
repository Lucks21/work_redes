#include <iostream>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <unordered_map>

std::mutex mtx;

class CuatroEnLinea {
public:
    CuatroEnLinea();
    void mostrarTablero();
    bool hacerJugada(int columna);
    bool comprobarGanador();
    char obtenerTurno() const;
    std::string obtenerTableroComoString() const;

private:
    std::vector<std::vector<char>> tablero;
    char turno;

    bool comprobarFilas();
    bool comprobarColumnas();
    bool comprobarDiagonales();
};

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

struct Partida {
    CuatroEnLinea juego;
    int jugador1;
    int jugador2;
    bool turnoJugador1;
};

std::vector<Partida> partidas;
std::unordered_map<int, int> clienteAPartida;

void enviarMensaje(int clienteSocket, const std::string& mensaje) {
    send(clienteSocket, mensaje.c_str(), mensaje.length(), 0);
}

void manejarCliente(int clienteSocket, int jugadorID) {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Jugador " << jugadorID << " ha entrado al juego.\n";

    // Emparejar jugadores
    if (partidas.empty() || partidas.back().jugador2 != -1) {
        // Crear una nueva partida
        Partida nuevaPartida;
        nuevaPartida.jugador1 = clienteSocket;
        nuevaPartida.jugador2 = -1;
        nuevaPartida.turnoJugador1 = true;
        partidas.push_back(nuevaPartida);
        clienteAPartida[clienteSocket] = partidas.size() - 1;
        enviarMensaje(clienteSocket, "Esperando a otro jugador...\n");
    } else {
        // Unir al cliente a la última partida creada
        int indicePartida = partidas.size() - 1;
        partidas[indicePartida].jugador2 = clienteSocket;
        clienteAPartida[clienteSocket] = indicePartida;
        enviarMensaje(partidas[indicePartida].jugador1, "Jugador 2 se ha unido. Empieza el juego.\n");
        enviarMensaje(partidas[indicePartida].jugador2, "Te has unido. Empieza el juego.\n");
        std::string tableroString = partidas[indicePartida].juego.obtenerTableroComoString();
        enviarMensaje(partidas[indicePartida].jugador1, tableroString);
        enviarMensaje(partidas[indicePartida].jugador2, tableroString);
    }
    lock.unlock();

    // Manejar la partida
    char buffer[1024] = {0};
    bool ganador = false;
    int columna;

    while (!ganador) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(clienteSocket, buffer, 1024);
        if (valread > 0) {
            columna = std::stoi(buffer);
            lock.lock();
            int indicePartida = clienteAPartida[clienteSocket];
            Partida& partida = partidas[indicePartida];

            bool esTurnoJugador1 = partida.turnoJugador1 && clienteSocket == partida.jugador1;
            bool esTurnoJugador2 = !partida.turnoJugador1 && clienteSocket == partida.jugador2;

            if (esTurnoJugador1 || esTurnoJugador2) {
                if (partida.juego.hacerJugada(columna)) {
                    ganador = partida.juego.comprobarGanador();
                    partida.turnoJugador1 = !partida.turnoJugador1;
                    std::string tableroString = partida.juego.obtenerTableroComoString();
                    enviarMensaje(partida.jugador1, tableroString);
                    enviarMensaje(partida.jugador2, tableroString);
                } else {
                    enviarMensaje(clienteSocket, "Columna no válida. Inténtelo de nuevo.\n");
                }
            } else {
                enviarMensaje(clienteSocket, "No es tu turno. Espera tu turno.\n");
            }
            lock.unlock();
        }
    }
    close(clienteSocket);
}

int main(int argc, char const *argv[]) {
    int servidor_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int opt = 1;
    int addrlen = sizeof(direccion);

    if ((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fallo al crear el socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Fallo en setsockopt");
        exit(EXIT_FAILURE);
    }

    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(7777);

    if (bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Fallo en bind");
        exit(EXIT_FAILURE);
    }

    if (listen(servidor_fd, 3) < 0) {
        perror("Fallo en listen");
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> hilos;
    int jugadorID = 1;

    while ((nuevo_socket = accept(servidor_fd, (struct sockaddr *)&direccion, (socklen_t*)&addrlen)) >= 0) {
        hilos.emplace_back(manejarCliente, nuevo_socket, jugadorID++);
    }

    for (auto &hilo : hilos) {
        hilo.join();
    }

    return 0;
}