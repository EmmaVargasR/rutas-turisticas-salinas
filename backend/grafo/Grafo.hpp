#pragma once
#include "../estructuras/ArregloDinamico.hpp"
#include "../modelos/Vertice.hpp"
#include <cmath>
#include <ostream>
#include <optional>
#include <stdexcept>
#include <utility>

class Grafo {
    ArregloDinamico<Vertice> vertices_;

    static void validarLugar(const Lugar& lugar) {
        if (lugar.id <= 0) throw std::invalid_argument("El ID del lugar debe ser positivo");
        if (lugar.nombre.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument("El nombre no puede estar vacio");
        }
        if (lugar.categoria.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument("La categoria no puede estar vacia");
        }
    }
    static void validarPesos(double distancia, double tiempo) {
        if (!std::isfinite(distancia) || distancia <= 0 ||
            !std::isfinite(tiempo) || tiempo <= 0) {
            throw std::invalid_argument("Distancia y tiempo deben ser positivos y finitos");
        }
    }
    std::size_t indiceDe(int id) const {
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            if (vertices_.acceder(i).lugar.id == id) return i;
        }
        throw std::out_of_range("El lugar no existe");
    }
    static bool tieneDestino(const Vertice& vertice, int destino) {
        return vertice.conexiones.buscar([=](const Camino& c) {
            return c.destino == destino;
        }) != nullptr;
    }

public:
    Grafo() = default;
    Grafo(const Grafo&) = delete;
    Grafo& operator=(const Grafo&) = delete;
    Grafo(Grafo&&) noexcept = default;
    Grafo& operator=(Grafo&&) noexcept = default;

    std::size_t cantidadLugares() const noexcept { return vertices_.cantidad(); }
    const Vertice& verticeEn(std::size_t indice) const { return vertices_.acceder(indice); }

    // Las referencias y punteros de consulta solo se conservan hasta la siguiente
    // modificacion del grafo. Las conexiones almacenan IDs estables, no punteros.
    const Lugar* buscarLugar(int id) const {
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            if (vertices_.acceder(i).lugar.id == id) return &vertices_.acceder(i).lugar;
        }
        return nullptr;
    }
    const Camino* buscarCamino(int idCamino) const {
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            const auto* encontrado = vertices_.acceder(i).conexiones.buscar(
                [=](const Camino& c) { return c.id == idCamino; });
            if (encontrado) return encontrado;
        }
        return nullptr;
    }
    const Camino* consultarConexion(int origen, int destino) const {
        const auto& vertice = vertices_.acceder(indiceDe(origen));
        return vertice.conexiones.buscar([=](const Camino& c) { return c.destino == destino; });
    }

    void registrarLugar(Lugar lugar) {
        validarLugar(lugar);
        if (buscarLugar(lugar.id)) throw std::invalid_argument("El ID del lugar ya esta registrado");
        vertices_.insertar(Vertice(std::move(lugar)));
    }
    // El ID es estable: se edita el contenido sin cambiar sus conexiones.
    void editarLugar(Lugar lugar) {
        validarLugar(lugar);
        vertices_.acceder(indiceDe(lugar.id)).lugar = std::move(lugar);
    }
    void eliminarLugar(int id) {
        const auto indice = indiceDe(id);
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            vertices_.acceder(i).conexiones.eliminarSi(
                [=](const Camino& c) { return c.destino == id; });
        }
        // El movimiento del arreglo conserva las listas de los vertices restantes.
        vertices_.eliminar(indice);
    }

    void conectar(int idCamino, int origen, int destino, double distanciaMetros,
                  double tiempoMinutos, bool bidireccional, bool habilitado = true, const std::string& nota = "") {
        if (nota.size() > 2000) throw std::invalid_argument("La nota del camino es demasiado larga");
        if (idCamino <= 0) throw std::invalid_argument("El ID del camino debe ser positivo");
        if (origen == destino) throw std::invalid_argument("Un camino necesita dos lugares distintos");
        validarPesos(distanciaMetros, tiempoMinutos);
        auto& desde = vertices_.acceder(indiceDe(origen));
        auto& hasta = vertices_.acceder(indiceDe(destino));
        if (buscarCamino(idCamino)) throw std::invalid_argument("El ID del camino ya existe");
        // La primera version admite un unico camino logico por pareja de lugares,
        // incluso si el intento nuevo tiene el sentido contrario.
        if (tieneDestino(desde, destino) || tieneDestino(hasta, origen)) {
            throw std::invalid_argument("Ya existe un camino entre esos lugares");
        }
        desde.conexiones.insertar({idCamino, destino, distanciaMetros, tiempoMinutos,
                                   bidireccional, habilitado, nota});
        if (bidireccional) {
            try {
                hasta.conexiones.insertar({idCamino, origen, distanciaMetros, tiempoMinutos,
                                           true, habilitado, nota});
            } catch (...) {
                // Si falla la segunda reserva, no dejamos un camino a medio crear.
                desde.conexiones.eliminarSi([=](const Camino& c) { return c.id == idCamino; });
                throw;
            }
        }
    }

    // Cambiar extremos o sentidos requiere eliminar y volver a crear el camino.
    void editarCamino(int idCamino, double distanciaMetros, double tiempoMinutos, bool habilitado,
                      const std::optional<std::string>& nota = std::nullopt) {
        if (nota && nota->size() > 2000) throw std::invalid_argument("La nota del camino es demasiado larga");
        validarPesos(distanciaMetros, tiempoMinutos);
        if (!buscarCamino(idCamino)) throw std::out_of_range("El camino no existe");
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            vertices_.acceder(i).conexiones.recorrer([=](Camino& c) {
                if (c.id == idCamino) {
                    c.distanciaMetros = distanciaMetros;
                    c.tiempoMinutos = tiempoMinutos;
                    c.habilitado = habilitado;
                    if (nota) c.nota = *nota;
                }
            });
        }
    }
    void eliminarCamino(int idCamino) {
        if (!buscarCamino(idCamino)) throw std::out_of_range("El camino no existe");
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            vertices_.acceder(i).conexiones.eliminarSi(
                [=](const Camino& c) { return c.id == idCamino; });
        }
    }

    std::size_t cantidadArcos() const {
        std::size_t total = 0;
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            total += vertices_.acceder(i).conexiones.cantidad();
        }
        return total;
    }
    std::size_t cantidadCaminos() const {
        std::size_t total = 0;
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            const auto& vertice = vertices_.acceder(i);
            vertice.conexiones.recorrer([&](const Camino& c) {
                if (!c.bidireccional || vertice.lugar.id < c.destino) ++total;
            });
        }
        return total;
    }
    void mostrar(std::ostream& salida) const {
        for (std::size_t i = 0; i < vertices_.cantidad(); ++i) {
            const auto& vertice = vertices_.acceder(i);
            salida << vertice.lugar.id << " | " << vertice.lugar.nombre
                   << (vertice.lugar.activo ? "" : " [inactivo]") << '\n';
            vertice.conexiones.recorrer([&](const Camino& c) {
                const auto* destino = buscarLugar(c.destino);
                salida << "  -> " << destino->nombre << " | " << c.distanciaMetros
                       << " m | " << c.tiempoMinutos << " min | camino " << c.id
                       << (c.habilitado ? "" : " [cerrado]") << '\n';
            });
        }
    }
};
