#pragma once
#include "Grafo.hpp"
#include "../modelos/ResultadoRuta.hpp"
#include <limits>

class AlgoritmosRutas {
    struct Estado {
        long double costo = std::numeric_limits<long double>::infinity();
        std::size_t anterior = std::numeric_limits<std::size_t>::max();
        bool procesado = false;
    };
    static std::size_t indiceDe(const Grafo& grafo, int id) {
        for (std::size_t i = 0; i < grafo.cantidadLugares(); ++i) {
            if (grafo.verticeEn(i).lugar.id == id) return i;
        }
        throw std::out_of_range("El lugar de origen o destino no existe");
    }

public:
    // Seleccion lineal del minimo, sin contenedores STL. Resolver cada ID vecino
    // tambien es lineal: complejidad total O(V^2 + V*E), espacio auxiliar O(V).
    static ResultadoRuta dijkstra(const Grafo& grafo, int origen, int destino, CriterioRuta criterio) {
        if (criterio != CriterioRuta::Distancia && criterio != CriterioRuta::Tiempo) {
            throw std::invalid_argument("Criterio de ruta no valido");
        }
        const auto inicio = indiceDe(grafo, origen);
        const auto fin = indiceDe(grafo, destino);
        if (!grafo.verticeEn(inicio).lugar.activo || !grafo.verticeEn(fin).lugar.activo) {
            throw std::invalid_argument("El origen y el destino deben ser lugares activos");
        }
        ResultadoRuta resultado;
        resultado.criterio = criterio;
        const auto n = grafo.cantidadLugares();
        ArregloDinamico<Estado> estados(n);
        for (std::size_t i = 0; i < n; ++i) estados.insertar(Estado{});
        estados.acceder(inicio).costo = 0;

        for (std::size_t paso = 0; paso < n; ++paso) {
            std::size_t actual = n;
            long double minimo = std::numeric_limits<long double>::infinity();
            for (std::size_t i = 0; i < n; ++i) {
                const auto& e = estados.acceder(i);
                if (!e.procesado && e.costo < minimo) {
                    minimo = e.costo;
                    actual = i;
                }
            }
            if (actual == n) break; // No quedan vertices alcanzables.
            estados.acceder(actual).procesado = true;
            if (actual == fin) break;

            grafo.verticeEn(actual).conexiones.recorrer([&](const Camino& camino) {
                if (!camino.habilitado) return;
                const auto vecino = indiceDe(grafo, camino.destino);
                auto& siguiente = estados.acceder(vecino);
                if (siguiente.procesado || !grafo.verticeEn(vecino).lugar.activo) return;
                const long double peso = criterio == CriterioRuta::Distancia
                    ? camino.distanciaMetros : camino.tiempoMinutos;
                const auto candidato = minimo + peso;
                if (candidato < siguiente.costo) {
                    siguiente.costo = candidato;
                    siguiente.anterior = actual;
                }
            });
        }
        if (!estados.acceder(fin).procesado) return resultado;

        // Reconstruir con indices temporales; devolver solo IDs estables.
        ArregloDinamico<std::size_t> inverso(n);
        auto actual = fin;
        while (true) {
            inverso.insertar(actual);
            if (actual == inicio) break;
            actual = estados.acceder(actual).anterior;
            if (actual >= n || inverso.cantidad() >= n) {
                throw std::logic_error("No se pudo reconstruir la ruta");
            }
        }
        long double distancia = 0;
        long double tiempo = 0;
        for (std::size_t i = inverso.cantidad(); i > 0; --i) {
            const auto id = grafo.verticeEn(inverso.acceder(i - 1)).lugar.id;
            if (!resultado.lugares.vacio()) {
                const auto previo = resultado.lugares.acceder(resultado.lugares.cantidad() - 1);
                const auto* camino = grafo.consultarConexion(previo, id);
                if (!camino) throw std::logic_error("La ruta contiene una conexion inexistente");
                resultado.tramos.insertar({camino->id, previo, id, camino->distanciaMetros, camino->tiempoMinutos});
                distancia += camino->distanciaMetros;
                tiempo += camino->tiempoMinutos;
            }
            resultado.lugares.insertar(id);
        }
        const auto limite = std::numeric_limits<double>::max();
        if (!std::isfinite(distancia) || !std::isfinite(tiempo) || distancia > limite || tiempo > limite) {
            throw std::overflow_error("Los totales de la ruta superan el limite numerico admitido");
        }
        resultado.distanciaTotal = static_cast<double>(distancia);
        resultado.tiempoTotal = static_cast<double>(tiempo);
        resultado.encontrada = true;
        return resultado;
    }
};
