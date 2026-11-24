#pragma once
#include "../estructuras/ArregloDinamico.hpp"

enum class CriterioRuta { Distancia, Tiempo };

struct TramoRuta {
    int idCamino = 0;
    int origen = 0;
    int destino = 0;
    double distanciaMetros = 0;
    double tiempoMinutos = 0;
};

struct ResultadoRuta {
    bool encontrada = false;
    CriterioRuta criterio = CriterioRuta::Distancia;
    ArregloDinamico<int> lugares;
    ArregloDinamico<TramoRuta> tramos;
    double distanciaTotal = 0;
    double tiempoTotal = 0;
};
