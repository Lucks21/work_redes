#include <iostream>
#include <vector>

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