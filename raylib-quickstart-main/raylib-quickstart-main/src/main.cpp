/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#include <raylib.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

class Bloque {
public:
    Rectangle rect;
    Color color;
    bool esHielo;
    bool destruible;

    Bloque(float x, float y, float width, float height, Color col, bool hielo = false, bool destruct = true);
    void dibujar() const; // Added const qualifier
};

class Enemigo {
public:
    Rectangle rect;
    float velocidad;
    Color color;

    Enemigo(float x, float y);
    void mover(const std::vector<Bloque>& bloques);
    void dibujar() const; // Added const qualifier
};

class Pengo {
private:
    Rectangle rect;
    float velocidad;
    int vidas;
    bool puedeEmpujar;
    bool empujando;
    Direction ultimaDireccion;

    enum class Direction {
        DERECHA, IZQUIERDA, ARRIBA, ABAJO, NINGUNA
    };

public:
    Pengo(float x, float y);
    void mover(std::vector<Bloque>& bloques);
    void dibujar() const;
    Rectangle getRect() const;
    int getVidas() const;
    void perderVida();
    void empujarBloque(std::vector<Bloque>& bloques);
    bool intentarRomperBloque(std::vector<Bloque>& bloques);
    void actualizarDireccion();
};

// Implementación de las técnicas de Pengo
void Pengo::mover(std::vector<Bloque>& bloques) {
    Rectangle nuevaPosicion = rect;
    actualizarDireccion();

    // Movimiento con flechas
    if (IsKeyDown(KEY_RIGHT)) {
        nuevaPosicion.x += velocidad;
        ultimaDireccion = Direction::DERECHA;
    }
    if (IsKeyDown(KEY_LEFT)) {
        nuevaPosicion.x -= velocidad;
        ultimaDireccion = Direction::IZQUIERDA;
    }
    if (IsKeyDown(KEY_DOWN)) {
        nuevaPosicion.y += velocidad;
        ultimaDireccion = Direction::ABAJO;
    }
    if (IsKeyDown(KEY_UP)) {
        nuevaPosicion.y -= velocidad;
        ultimaDireccion = Direction::ARRIBA;
    }

    // Verificar colisiones con bloques
    bool colision = false;
    for (auto& bloque : bloques) {
        if (CheckCollisionRecs(nuevaPosicion, bloque.rect)) {
            colision = true;

            // Lógica de empuje de bloques
            if (IsKeyDown(KEY_SPACE) && bloque.destruible) {
                empujarBloque(bloques);
                break;
            }

            // Técnica de romper bloques de hielo
            if (IsKeyDown(KEY_X) && bloque.esHielo) {
                if (intentarRomperBloque(bloques)) {
                    break;
                }
            }
        }
    }

    // Actualizar posición si no hay colisión
    if (!colision) {
        rect = nuevaPosicion;
    }
}

void Pengo::empujarBloque(std::vector<Bloque>& bloques) {
    if (!puedeEmpujar) return;

    Rectangle posicionEmpuje = rect;
    switch (ultimaDireccion) {
    case Direction::DERECHA: posicionEmpuje.x += rect.width; break;
    case Direction::IZQUIERDA: posicionEmpuje.x -= rect.width; break;
    case Direction::ARRIBA: posicionEmpuje.y -= rect.height; break;
    case Direction::ABAJO: posicionEmpuje.y += rect.height; break;
    default: return;
    }

    // Buscar bloque a empujar
    for (auto& bloque : bloques) {
        if (CheckCollisionRecs(posicionEmpuje, bloque.rect) && bloque.destruible) {
            // Mover bloque en la dirección de empuje
            switch (ultimaDireccion) {
            case Direction::DERECHA: bloque.rect.x += velocidad; break;
            case Direction::IZQUIERDA: bloque.rect.x -= velocidad; break;
            case Direction::ARRIBA: bloque.rect.y -= velocidad; break;
            case Direction::ABAJO: bloque.rect.y += velocidad; break;
            default: break;
            }
            break;
        }
    }
}

bool Pengo::intentarRomperBloque(std::vector<Bloque>& bloques) {
    // Encontrar bloque de hielo adyacente
    for (auto it = bloques.begin(); it != bloques.end(); ) {
        if (it->esHielo && CheckCollisionRecs(rect, it->rect)) {
            // Romper bloque de hielo
            it = bloques.erase(it);
            return true;
        }
        else {
            ++it;
        }
    }
    return false;
}

void Pengo::actualizarDireccion() {
    // Lógica para actualizar la dirección del personaje
    if (IsKeyDown(KEY_RIGHT)) ultimaDireccion = Direction::DERECHA;
    else if (IsKeyDown(KEY_LEFT)) ultimaDireccion = Direction::IZQUIERDA;
    else if (IsKeyDown(KEY_UP)) ultimaDireccion = Direction::ARRIBA;
    else if (IsKeyDown(KEY_DOWN)) ultimaDireccion = Direction::ABAJO;
}

void Pengo::dibujar() const {
    // Dibujar personaje con color diferente según la dirección
    Color colorPengo = BLUE;
    switch (ultimaDireccion) {
    case Direction::DERECHA: colorPengo = BLUE; break;
    case Direction::IZQUIERDA: colorPengo = DARKBLUE; break;
    case Direction::ARRIBA: colorPengo = SKYBLUE; break;
    case Direction::ABAJO: colorPengo = BLUE; break;
    default: break;
    }

    DrawRectangleRec(rect, colorPengo);

    // Dibujar vidas
    for (int i = 0; i < vidas; i++) {
        DrawRectangle(10 + i * 40, 10, 30, 30, GREEN);
    }
}

class Juego {
private:
    static const int anchoPantalla = 800;
    static const int altoPantalla = 600;

    Pengo pengo;
    std::vector<Bloque> bloques;
    std::vector<Enemigo> enemigos;
    int puntaje;
    bool juegoTerminado;

    void generarBloques();
    void generarEnemigos();

public:
    Juego();
    void actualizar();
    void dibujar();
    void ejecutar();
};

// Bloque method implementations
Bloque::Bloque(float x, float y, float width, float height, Color col, bool hielo, bool destruct)
    : rect({
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(width),
        static_cast<float>(height)
        }),
    color(col),
    esHielo(hielo),
    destruible(destruct) {}

void Bloque::dibujar() const {
    DrawRectangleRec(rect, color);
}

// Enemigo method implementations
Enemigo::Enemigo(float x, float y)
    : rect({
        static_cast<float>(x),
        static_cast<float>(y),
        32.0f,
        32.0f
        }),
    velocidad(2.0f),
    color(RED) {}

void Enemigo::mover(const std::vector<Bloque>& bloques) {
    // Movimiento aleatorio simple
    int direccion = rand() % 4;

    Rectangle nuevaPosicion = rect;
    switch (direccion) {
    case 0: nuevaPosicion.x += static_cast<float>(velocidad); break; // Derecha
    case 1: nuevaPosicion.x -= static_cast<float>(velocidad); break; // Izquierda
    case 2: nuevaPosicion.y += static_cast<float>(velocidad); break; // Abajo
    case 3: nuevaPosicion.y -= static_cast<float>(velocidad); break; // Arriba
    }

    // Verificar colisiones con bloques
    bool colision = false;
    for (const auto& bloque : bloques) {
        if (CheckCollisionRecs(nuevaPosicion, bloque.rect)) {
            colision = true;
            break;
        }
    }

    // Actualizar posición si no hay colisión
    if (!colision) {
        rect = nuevaPosicion;
    }
}

void Enemigo::dibujar() const {
    DrawRectangleRec(rect, color);
}

// Pengo method implementations
Pengo::Pengo(float x, float y)
    : rect({
        static_cast<float>(x),
        static_cast<float>(y),
        32.0f,
        32.0f
        }),
    velocidad(5.0f),
    vidas(3),
    puedeEmpujar(false) {}

void Pengo::mover(const std::vector<Bloque>& bloques) {
    Rectangle nuevaPosicion = rect;

    // Movimiento con flechas
    if (IsKeyDown(KEY_RIGHT)) nuevaPosicion.x += static_cast<float>(velocidad);
    if (IsKeyDown(KEY_LEFT))  nuevaPosicion.x -= static_cast<float>(velocidad);
    if (IsKeyDown(KEY_DOWN))  nuevaPosicion.y += static_cast<float>(velocidad);
    if (IsKeyDown(KEY_UP))    nuevaPosicion.y -= static_cast<float>(velocidad);

    // Verificar colisiones con bloques
    bool colision = false;
    for (const auto& bloque : bloques) {
        if (CheckCollisionRecs(nuevaPosicion, bloque.rect)) {
            colision = true;

            // Lógica de empuje de bloques
            if (puedeEmpujar && bloque.destruible) {
                // Implementar lógica de empuje de bloques
            }
            break;
        }
    }

    // Actualizar posición si no hay colisión
    if (!colision) {
        rect = nuevaPosicion;
    }
}

void Pengo::dibujar() const {
    DrawRectangleRec(rect, BLUE);

    // Dibujar vidas
    for (int i = 0; i < vidas; i++) {
        DrawRectangle(10 + i * 40, 10, 30, 30, GREEN);
    }
}

Rectangle Pengo::getRect() const { return rect; }
int Pengo::getVidas() const { return vidas; }
void Pengo::perderVida() { vidas--; }

// Juego method implementations
Juego::Juego()
    : pengo(static_cast<float>(anchoPantalla / 2.0f), static_cast<float>(altoPantalla / 2.0f)),
    puntaje(0),
    juegoTerminado(false) {

    srand(time(nullptr));
    InitWindow(anchoPantalla, altoPantalla, "Pengo - Juego Completo");
    SetTargetFPS(60);

    generarBloques();
    generarEnemigos();
}

void Juego::generarBloques() {
    // Generar bloques de hielo
    for (int i = 0; i < 5; i++) {
        float x = static_cast<float>(rand() % (anchoPantalla - 50));
        float y = static_cast<float>(rand() % (altoPantalla - 50));
        bloques.push_back(Bloque(x, y, 32.0f, 32.0f, LIGHTGRAY, true));
    }

    // Generar paredes
    bloques.push_back(Bloque(0, 0, static_cast<float>(anchoPantalla), 32, DARKGRAY, false, false)); // Arriba
    bloques.push_back(Bloque(0, static_cast<float>(altoPantalla - 32), static_cast<float>(anchoPantalla), 32, DARKGRAY, false, false)); // Abajo
    bloques.push_back(Bloque(0, 0, 32, static_cast<float>(altoPantalla), DARKGRAY, false, false)); // Izquierda
    bloques.push_back(Bloque(static_cast<float>(anchoPantalla - 32), 0, 32, static_cast<float>(altoPantalla), DARKGRAY, false, false)); // Derecha
}

void Juego::generarEnemigos() {
    for (int i = 0; i < 3; i++) {
        float x = static_cast<float>(rand() % (anchoPantalla - 50));
        float y = static_cast<float>(rand() % (altoPantalla - 50));
        enemigos.push_back(Enemigo(x, y));
    }
}

void Juego::actualizar() {
    if (!juegoTerminado) {
        // Movimiento de Pengo
        pengo.mover(bloques);

        // Movimiento de enemigos
        for (auto& enemigo : enemigos) {
            enemigo.mover(bloques);

            // Colisión con Pengo
            if (CheckCollisionRecs(pengo.getRect(), enemigo.rect)) {
                pengo.perderVida();

                if (pengo.getVidas() <= 0) {
                    juegoTerminado = true;
                }
            }
        }
    }
}

void Juego::dibujar() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (!juegoTerminado) {
        // Dibujar bloques
        for (const auto& bloque : bloques) {
            bloque.dibujar();
        }

        // Dibujar enemigos
        for (const auto& enemigo : enemigos) {
            enemigo.dibujar();
        }

        // Dibujar Pengo
        pengo.dibujar();

        // Mostrar puntaje
        DrawText(TextFormat("Puntaje: %d", puntaje), 10, 50, 20, BLACK);
    }
    else {
        // Pantalla de Game Over
        DrawText("GAME OVER", anchoPantalla / 2 - MeasureText("GAME OVER", 40) / 2,
            altoPantalla / 2 - 20, 40, RED);
        DrawText(TextFormat("Puntaje Final: %d", puntaje),
            anchoPantalla / 2 - MeasureText(TextFormat("Puntaje Final: %d", puntaje), 20) / 2,
            altoPantalla / 2 + 50, 20, BLACK);
    }

    EndDrawing();
}

void Juego::ejecutar() {
    while (!WindowShouldClose()) {
        actualizar();
        dibujar();
    }

    CloseWindow();
}

int main() {
    Juego juego;
    juego.ejecutar();
    return 0;
}