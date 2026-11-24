#pragma once
#include "Lugar.hpp"
#include "Camino.hpp"
#include "../estructuras/ListaEnlazada.hpp"
#include <utility>

struct Vertice {
    Lugar lugar;
    ListaEnlazada<Camino> conexiones;

    Vertice() = default;
    explicit Vertice(Lugar dato) : lugar(std::move(dato)) {}
    Vertice(const Vertice&) = delete;
    Vertice& operator=(const Vertice&) = delete;
    Vertice(Vertice&&) noexcept = default;
    Vertice& operator=(Vertice&&) noexcept = default;
};
