#include <iostream>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

class cuatroEnLinea {
public:
    cuatroEnLinea();
    void mostrarTablero();
    bool hacerJugada(int columna);
    bool comprobarGanador();
    char obtenerTurno() const;

private:
    std::vector<std::vector<char>> tablero;
    char turno;

    bool comprobarFilas();
    bool comprobarColumnas();
    bool comprobarDiagonales();
};

cuatroEnLinea::cuatroEnLinea() : tablero(6, std::vector<char>(7, ' ')), turno('C') {}

void cuatroEnLinea::mostrarTablero() {
    for (const auto& fila : tablero) {
        for (const auto& celda : fila) {
            std::cout << "|" << celda;
        }
        std::cout << "|\n";
    }
    std::cout << "---------------\n";
    std::cout << " 1 2 3 4 5 6 7\n";
}

bool cuatroEnLinea::hacerJugada(int columna) {
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

bool cuatroEnLinea::comprobarGanador() {
    return comprobarFilas() || comprobarColumnas() || comprobarDiagonales();
}

char cuatroEnLinea::obtenerTurno() const {
    return turno;
}

bool cuatroEnLinea::comprobarFilas() {
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

bool cuatroEnLinea::comprobarColumnas() {
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

bool cuatroEnLinea::comprobarDiagonales() {
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
void manejarCliente(int clienteSocket){
    cuatroEnLinea juego;
    char buffer[1024] = {0};
    int columna;
    bool ganador = false;

    send(clienteSocket, "Conectando al servidor...\n", strlen("Conectado al servidor CUATRO EN LINEA\n"),0);
    while (!ganador)
    {
        memset(buffer,0,sizeof(buffer));
        read(clienteSocket,buffer,1024);
        columna = std::stoi(buffer);
        if(juego.hacerJugada(columna)){
            ganador = juego.comprobarGanador();
            juego.mostrarTablero();
        }
        send(clienteSocket,buffer,strlen(buffer),0);
    }
    close(clienteSocket);
}
int main(int argc, char const *argv[]){
    int servidor_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int opt = 1;
    int addrlen = sizeof(direccion);
    if ((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Fallo al crear el socket");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt))){
        perror("Fallo en setsockopt");
        exit(EXIT_FAILURE);
    }

    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(7777);
    if(bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0 ){
        perror("Fallo en bind");
        exit(EXIT_FAILURE);
    }
    if(listen(servidor_fd, 3)< 0){
        perror("Fallo en listen");
        exit(EXIT_FAILURE);
    }
    std::vector<std::thread > hilos;
    while ((nuevo_socket = accept(servidor_fd,(struct sockaddr *)&direccion,(socklen_t*)&addrlen)) >= 0){
        hilos.emplace_back(manejarCliente,nuevo_socket);
    }
    for (auto &hilo : hilos){
        hilo.join();
    }
    return 0;
}