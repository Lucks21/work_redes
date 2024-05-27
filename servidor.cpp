/*
 * servidor.cpp
 * 
 * g++ -std=c++11 servidor.cpp -o servidor -lpthread
 *
 */

#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // hton*()
#include <string.h>     // memset()
#include <unistd.h> 
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>

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

void manejarCliente(int socket_cliente, struct sockaddr_in direccionCliente) {
    std::unique_lock<std::mutex> lock(mtx);
    static int jugadorID = 1;
    int jugadorActual = jugadorID++;
    
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);
    cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Jugador " << jugadorActual << " ha entrado al juego." << endl;

    // Emparejar jugadores
    if (partidas.empty() || partidas.back().jugador2 != -1) {
        // Crear una nueva partida
        Partida nuevaPartida;
        nuevaPartida.jugador1 = socket_cliente;
        nuevaPartida.jugador2 = -1;
        nuevaPartida.turnoJugador1 = true;
        partidas.push_back(nuevaPartida);
        clienteAPartida[socket_cliente] = partidas.size() - 1;
        enviarMensaje(socket_cliente, "Esperando a otro jugador...\n");
    } else {
        // Unir al cliente a la última partida creada
        int indicePartida = partidas.size() - 1;
        partidas[indicePartida].jugador2 = socket_cliente;
        clienteAPartida[socket_cliente] = indicePartida;
        enviarMensaje(partidas[indicePartida].jugador1, "Jugador 2 se ha unido. Empieza el juego.\n");
        enviarMensaje(partidas[indicePartida].jugador2, "Te has unido. Empieza el juego.\n");
        std::string tableroString = partidas[indicePartida].juego.obtenerTableroComoString();
        enviarMensaje(partidas[indicePartida].jugador1, tableroString);
        enviarMensaje(partidas[indicePartida].jugador2, tableroString);
    }
    lock.unlock();

    // Manejar la partida
    char buffer[1024];
    bool ganador = false;
    bool empate = false;
    int columna;

    while (!ganador && !empate) {
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(socket_cliente, buffer, 1024, 0);
        if (valread > 0) {
            columna = std::stoi(buffer);
            lock.lock();
            int indicePartida = clienteAPartida[socket_cliente];
            Partida& partida = partidas[indicePartida];

            bool esTurnoJugador1 = partida.turnoJugador1 && socket_cliente == partida.jugador1;
            bool esTurnoJugador2 = !partida.turnoJugador1 && socket_cliente == partida.jugador2;

            if (esTurnoJugador1 || esTurnoJugador2) {
                if (partida.juego.hacerJugada(columna)) {
                    ganador = partida.juego.comprobarGanador();
                    empate = partida.juego.comprobarEmpate();
                    partida.turnoJugador1 = !partida.turnoJugador1;
                    std::string tableroString = partida.juego.obtenerTableroComoString();
                    enviarMensaje(partida.jugador1, tableroString);
                    enviarMensaje(partida.jugador2, tableroString);
                    
                    if (ganador) {
                        std::string mensajeGanador = "Jugador " + std::to_string(esTurnoJugador1 ? 1 : 2) + " ha ganado!\n";
                        enviarMensaje(partida.jugador1, mensajeGanador);
                        enviarMensaje(partida.jugador2, mensajeGanador);
                    } else if (empate) {
                        std::string mensajeEmpate = "El juego ha terminado en empate.\n";
                        enviarMensaje(partida.jugador1, mensajeEmpate);
                        enviarMensaje(partida.jugador2, mensajeEmpate);
                    }
                } else {
                    enviarMensaje(socket_cliente, "Columna no válida. Inténtelo de nuevo.\n");
                }
            } else {
                enviarMensaje(socket_cliente, "No es tu turno. Espera tu turno.\n");
            }
            lock.unlock();
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

    cout << "Creating listening socket ...\n";
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Error creating listening socket\n";
        exit(EXIT_FAILURE);
    }

    cout << "Configuring socket address structure ...\n";
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family      = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port        = htons(port);

    cout << "Binding socket ...\n";
    if (bind(socket_server, (struct sockaddr *) &direccionServidor, sizeof(direccionServidor)) < 0) {
        cout << "Error calling bind()\n";
        exit(EXIT_FAILURE);
    }

    cout << "Calling listening ...\n";
    if (listen(socket_server, 1024) < 0) {
        cout << "Error calling listen()\n";
        exit(EXIT_FAILURE);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    cout << "Waiting client request ...\n";

    vector<thread> hilos;

    while (true) {
        int socket_cliente;
        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size)) < 0) {
            cout << "Error calling accept()\n";
            exit(EXIT_FAILURE);
        }

        hilos.emplace_back(manejarCliente, socket_cliente, direccionCliente);
    }

    for (auto &hilo : hilos) {
        hilo.join();
    }

    return 0;
}