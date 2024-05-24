#include "cuatroEnLinea.cpp"

int main() {
    cuatroEnLinea juego;
    int columna;
    bool ganador = false;

    std::cout << "Bienvenido al juego Cuatro en Línea!\n";

    while (!ganador) {
        juego.mostrarTablero();
        std::cout << "Turno del jugador " << juego.obtenerTurno() << ". Introduzca la columna (1-7): ";
        std::cin >> columna;
        
        if (juego.hacerJugada(columna)) {
            ganador = juego.comprobarGanador();
        }
    }

    juego.mostrarTablero();
    std::cout << "El jugador " << (juego.obtenerTurno() == 'C' ? 'S' : 'C') << " ha ganado!\n";

    return 0;
}