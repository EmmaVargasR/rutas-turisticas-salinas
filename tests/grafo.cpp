#include "grafo/Grafo.hpp"
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}
template <typename Error, typename Funcion>
void rechazar(Funcion ejecutar) {
    try { ejecutar(); }
    catch (const Error&) { return; }
    throw std::runtime_error("La operacion debio rechazarse");
}
Lugar lugar(int id) {
    return {id, "Punto de prueba " + std::to_string(id), "Dato ficticio", "Prueba", true};
}

void probarLugares() {
    Grafo g;
    comprobar(g.cantidadLugares() == 0 && !g.buscarLugar(1), "Grafo vacio");
    rechazar<std::out_of_range>([&] { g.eliminarLugar(1); });
    rechazar<std::invalid_argument>([&] { g.registrarLugar(lugar(0)); });
    auto invalido = lugar(1);
    invalido.nombre = " \t\n";
    rechazar<std::invalid_argument>([&] { g.registrarLugar(invalido); });
    invalido = lugar(1);
    invalido.categoria = "";
    rechazar<std::invalid_argument>([&] { g.registrarLugar(invalido); });
    g.registrarLugar(lugar(10));
    rechazar<std::invalid_argument>([&] { g.registrarLugar(lugar(10)); });
    auto editado = lugar(10);
    editado.nombre = "Punto editado";
    editado.activo = false;
    g.editarLugar(editado);
    comprobar(g.buscarLugar(10)->nombre == "Punto editado" && !g.buscarLugar(10)->activo, "Editar lugar");
    rechazar<std::out_of_range>([&] { g.editarLugar(lugar(99)); });
    g.eliminarLugar(10);
    comprobar(g.cantidadLugares() == 0, "Eliminar unico lugar");
}

void probarCaminos() {
    Grafo g;
    for (int id : {10, 20, 30, 40}) g.registrarLugar(lugar(id));
    g.conectar(100, 10, 20, 300, 5, true);
    g.conectar(200, 20, 30, 120, 2, false);
    comprobar(g.cantidadCaminos() == 2 && g.cantidadArcos() == 3, "Contar caminos y arcos");
    comprobar(g.consultarConexion(10, 20)->id == 100 && g.consultarConexion(20, 10)->id == 100,
              "Camino bidireccional");
    comprobar(g.consultarConexion(20, 30) && !g.consultarConexion(30, 20), "Camino unidireccional");
    rechazar<std::invalid_argument>([&] { g.conectar(300, 20, 10, 1, 1, false); });
    rechazar<std::invalid_argument>([&] { g.conectar(300, 30, 20, 1, 1, false); });
    rechazar<std::invalid_argument>([&] { g.conectar(100, 30, 40, 1, 1, false); });
    rechazar<std::invalid_argument>([&] { g.conectar(300, 10, 10, 1, 1, false); });
    rechazar<std::invalid_argument>([&] { g.conectar(-1, 10, 40, 1, 1, false); });
    rechazar<std::out_of_range>([&] { g.conectar(300, 10, 99, 1, 1, false); });
    for (double peso : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::quiet_NaN()}) {
        rechazar<std::invalid_argument>([&] { g.conectar(300, 10, 40, peso, 1, false); });
        rechazar<std::invalid_argument>([&] { g.conectar(300, 10, 40, 1, peso, false); });
        rechazar<std::invalid_argument>([&] { g.editarCamino(100, peso, 1, true); });
    }
    comprobar(g.cantidadCaminos() == 2 && g.cantidadArcos() == 3, "Errores sin modificaciones parciales");
    g.editarCamino(100, 350, 6, false);
    const auto* ida = g.consultarConexion(10, 20);
    const auto* vuelta = g.consultarConexion(20, 10);
    comprobar(ida->distanciaMetros == 350 && vuelta->distanciaMetros == 350 &&
              ida->tiempoMinutos == 6 && vuelta->tiempoMinutos == 6 &&
              !ida->habilitado && !vuelta->habilitado, "Edicion sincronizada");
    auto editado = lugar(20);
    editado.activo = false;
    g.editarLugar(editado);
    comprobar(g.cantidadArcos() == 3, "Desactivar lugar conserva las conexiones registradas");
    std::ostringstream texto;
    g.mostrar(texto);
    comprobar(texto.str().find("[cerrado]") != std::string::npos &&
              texto.str().find("[inactivo]") != std::string::npos, "Representacion textual");
    g.eliminarCamino(100);
    comprobar(!g.consultarConexion(10, 20) && !g.consultarConexion(20, 10), "Eliminar ambos sentidos");
    g.conectar(100, 10, 20, 300, 5, true);
    g.eliminarLugar(20);
    comprobar(!g.buscarLugar(20) && g.cantidadArcos() == 0 && g.cantidadCaminos() == 0,
              "Eliminar conexiones entrantes y salientes");
    rechazar<std::out_of_range>([&] { g.eliminarCamino(200); });
    rechazar<std::out_of_range>([&] { g.editarCamino(999, 1, 1, true); });
    g.registrarLugar(lugar(20));
    g.conectar(400, 20, 40, 50, 1, false);
    comprobar(g.cantidadCaminos() == 1, "Reutilizar grafo tras eliminaciones");
}

void probarCrecimiento() {
    Grafo g;
    g.registrarLugar(lugar(10));
    g.registrarLugar(lugar(20));
    g.conectar(1, 10, 20, 10, 1, true);
    for (int id = 30; id <= 1000; id += 10) g.registrarLugar(lugar(id));
    g.conectar(2, 20, 1000, 20, 2, false);
    g.eliminarLugar(500);
    g.eliminarLugar(10);
    comprobar(g.consultarConexion(20, 1000)->id == 2 && g.cantidadArcos() == 1,
              "IDs y conexiones conservados tras crecer y desplazar");
    Grafo movido(std::move(g));
    comprobar(g.cantidadLugares() == 0 && movido.cantidadLugares() == 98, "Mover grafo");
    g.registrarLugar(lugar(1));
    g = std::move(movido);
    comprobar(g.buscarLugar(1000) && !g.buscarLugar(1), "Asignar grafo por movimiento");
    for (int id = 20; id <= 1000; id += 10) {
        if (id != 500) g.eliminarLugar(id);
    }
    comprobar(g.cantidadLugares() == 0 && g.cantidadArcos() == 0, "Vaciar grafo completo");
}

int main() {
    static_assert(std::is_nothrow_move_assignable<Vertice>::value);
    static_assert(!std::is_copy_constructible<Grafo>::value);
    try {
        probarLugares();
        std::cout << "OK: registro, validacion, edicion y eliminacion de lugares\n";
        probarCaminos();
        std::cout << "OK: sentidos, pesos, duplicados y limpieza de conexiones\n";
        probarCrecimiento();
        std::cout << "OK: grafo tras crecer, mover y eliminar vertices\n";
    } catch (const std::exception& error) {
        std::cerr << "FALLO: " << error.what() << '\n';
        return 1;
    }
}
