#pragma once
#include <string>

// Un camino bidireccional tiene dos arcos con el mismo ID.
// El origen se deduce del vertice que contiene este arco.
struct Camino {
    int id = 0;
    int destino = 0;
    double distanciaMetros = 0;
    double tiempoMinutos = 0;
    bool bidireccional = false;
    bool habilitado = true;
    std::string nota;
};
