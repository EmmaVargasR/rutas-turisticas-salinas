#include "grafo/AlgoritmosRutas.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}
template <typename Error, typename Funcion>
void rechazar(Funcion ejecutar) {
    try { ejecutar(); } catch (const Error&) { return; }
    throw std::runtime_error("Se esperaba rechazar la operacion");
}
Lugar lugar(int id) { return {id, "Punto ficticio " + std::to_string(id), "", "Prueba", true}; }
Grafo ejemplo() {
    Grafo g;
    for (int id : {10, 20, 30, 40, 50}) g.registrarLugar(lugar(id));
    g.conectar(1, 10, 20, 100, 10, true);
    g.conectar(2, 20, 40, 100, 10, false);
    g.conectar(3, 10, 30, 300, 2, false);
    g.conectar(4, 30, 40, 300, 2, true);
    g.conectar(5, 20, 30, 50, 50, false);
    return g;
}
void comprobarRuta(const Grafo& g, const ResultadoRuta& r, int origen, int destino) {
    comprobar(r.encontrada && !r.lugares.vacio(), "Ruta encontrada");
    comprobar(r.lugares.acceder(0) == origen && r.lugares.acceder(r.lugares.cantidad() - 1) == destino, "Extremos");
    comprobar(r.tramos.cantidad() + 1 == r.lugares.cantidad(), "Cantidad de tramos");
    double metros = 0, minutos = 0;
    for (std::size_t i = 0; i < r.tramos.cantidad(); ++i) {
        const auto& t = r.tramos.acceder(i);
        const auto* c = g.consultarConexion(t.origen, t.destino);
        comprobar(c && c->habilitado && c->id == t.idCamino, "Tramo existente y habilitado");
        comprobar(g.buscarLugar(t.origen)->activo && g.buscarLugar(t.destino)->activo, "Lugares activos");
        comprobar(t.origen == r.lugares.acceder(i) && t.destino == r.lugares.acceder(i + 1), "Orden de tramos");
        metros += c->distanciaMetros; minutos += c->tiempoMinutos;
    }
    comprobar(std::abs(metros - r.distanciaTotal) < 1e-9 && std::abs(minutos - r.tiempoTotal) < 1e-9, "Totales de ambas magnitudes");
}
void probarCriterios() {
    const auto g = ejemplo();
    const auto corta = AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Distancia);
    comprobarRuta(g, corta, 10, 40);
    comprobar(corta.lugares.acceder(1) == 20 && corta.distanciaTotal == 200 && corta.tiempoTotal == 20, "Ruta mas corta");
    const auto rapida = AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Tiempo);
    comprobarRuta(g, rapida, 10, 40);
    comprobar(rapida.lugares.acceder(1) == 30 && rapida.distanciaTotal == 600 && rapida.tiempoTotal == 4, "Ruta mas rapida");
    const auto relajada = AlgoritmosRutas::dijkstra(g, 10, 30, CriterioRuta::Distancia);
    comprobarRuta(g, relajada, 10, 30);
    comprobar(relajada.lugares.cantidad() == 3 && relajada.distanciaTotal == 150, "Actualizar predecesor de un vertice descubierto");
    const auto igual = AlgoritmosRutas::dijkstra(g, 10, 10, CriterioRuta::Tiempo);
    comprobarRuta(g, igual, 10, 10);
    comprobar(igual.tramos.vacio() && igual.distanciaTotal == 0 && igual.tiempoTotal == 0, "Origen igual al destino");
    comprobarRuta(g, AlgoritmosRutas::dijkstra(g, 20, 10, CriterioRuta::Distancia), 20, 10);
    for (int destino : {10, 50}) {
        const auto sinRuta = AlgoritmosRutas::dijkstra(g, 40, destino, CriterioRuta::Distancia);
        comprobar(!sinRuta.encontrada && sinRuta.lugares.vacio() && sinRuta.tramos.vacio(), "Sin ruta por sentido o desconexion");
    }
}
void probarCambiosYErrores() {
    Grafo vacio;
    rechazar<std::out_of_range>([&] { AlgoritmosRutas::dijkstra(vacio, 1, 2, CriterioRuta::Distancia); });
    auto g = ejemplo();
    rechazar<std::out_of_range>([&] { AlgoritmosRutas::dijkstra(g, 10, 999, CriterioRuta::Distancia); });
    rechazar<std::invalid_argument>([&] { AlgoritmosRutas::dijkstra(g, 10, 40, static_cast<CriterioRuta>(99)); });
    auto inactivo = lugar(20); inactivo.activo = false; g.editarLugar(inactivo);
    rechazar<std::invalid_argument>([&] { AlgoritmosRutas::dijkstra(g, 20, 40, CriterioRuta::Tiempo); });
    rechazar<std::invalid_argument>([&] { AlgoritmosRutas::dijkstra(g, 10, 20, CriterioRuta::Tiempo); });
    comprobar(AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Distancia).distanciaTotal == 600, "Excluir intermedio inactivo");
    g.editarCamino(3, 300, 2, false);
    comprobar(!AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Distancia).encontrada, "Excluir camino cerrado");
    g.editarLugar(lugar(20));
    comprobar(AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Tiempo).tiempoTotal == 20, "Recalcular tras reactivar");
    g.eliminarLugar(20);
    g.editarCamino(3, 300, 2, true);
    comprobarRuta(g, AlgoritmosRutas::dijkstra(g, 10, 40, CriterioRuta::Tiempo), 10, 40);
    Grafo grande;
    for (int id : {1, 2, 3}) grande.registrarLugar(lugar(id));
    grande.conectar(1, 1, 2, 1e308, 1, false);
    grande.conectar(2, 2, 3, 1e308, 1, false);
    rechazar<std::overflow_error>([&] { AlgoritmosRutas::dijkstra(grande, 1, 3, CriterioRuta::Tiempo); });
}
// Verificador independiente: enumera todos los caminos simples de grafos pequenos.
void enumerar(const Grafo& g, int actual, int destino, CriterioRuta criterio,
              bool (&visitados)[6], double costo, double& mejor) {
    if (actual == destino) { if (costo < mejor) mejor = costo; return; }
    visitados[actual - 1] = true;
    for (int siguiente = 1; siguiente <= 6; ++siguiente) {
        if (visitados[siguiente - 1] || !g.buscarLugar(siguiente)->activo) continue;
        const auto* camino = g.consultarConexion(actual, siguiente);
        if (camino && camino->habilitado) {
            enumerar(g, siguiente, destino, criterio, visitados,
                costo + (criterio == CriterioRuta::Distancia ? camino->distanciaMetros : camino->tiempoMinutos), mejor);
        }
    }
    visitados[actual - 1] = false;
}
void compararConEnumeracion() {
    std::mt19937 generador(20261115);
    for (int caso = 0; caso < 12; ++caso) {
        Grafo g;
        for (int id = 1; id <= 6; ++id) g.registrarLugar(lugar(id));
        int caminoId = 1;
        for (int a = 1; a <= 6; ++a) for (int b = a + 1; b <= 6; ++b) {
            if (generador() % 3 == 0) continue;
            const bool inverso = generador() % 2 == 0;
            g.conectar(caminoId++, inverso ? b : a, inverso ? a : b,
                1 + generador() % 100, 1 + generador() % 30, generador() % 2 == 0, generador() % 5 != 0);
        }
        auto intermedio = lugar(3); intermedio.activo = caso % 2 == 0; g.editarLugar(intermedio);
        for (int a = 1; a <= 6; ++a) for (int b = 1; b <= 6; ++b) {
            if (!g.buscarLugar(a)->activo || !g.buscarLugar(b)->activo) continue;
            for (auto criterio : {CriterioRuta::Distancia, CriterioRuta::Tiempo}) {
                bool visitados[6]{};
                double mejor = std::numeric_limits<double>::infinity();
                enumerar(g, a, b, criterio, visitados, 0, mejor);
                const auto ruta = AlgoritmosRutas::dijkstra(g, a, b, criterio);
                comprobar(ruta.encontrada == std::isfinite(mejor), "Alcanzabilidad frente a enumeracion");
                if (ruta.encontrada) {
                    comprobarRuta(g, ruta, a, b);
                    comprobar((criterio == CriterioRuta::Distancia ? ruta.distanciaTotal : ruta.tiempoTotal) == mejor, "Optimo frente a enumeracion independiente");
                }
            }
        }
    }
}
int main() {
    try {
        probarCriterios();
        std::cout << "OK: rutas distintas por distancia/tiempo y reconstruccion\n";
        probarCambiosYErrores();
        std::cout << "OK: cierres, inactivos, sentidos, eliminaciones y limites\n";
        compararConEnumeracion();
        std::cout << "OK: Dijkstra coincide con enumeracion exhaustiva en 12 grafos pequenos\n";
    } catch (const std::exception& error) {
        std::cerr << "FALLO: " << error.what() << '\n';
        return 1;
    }
}
