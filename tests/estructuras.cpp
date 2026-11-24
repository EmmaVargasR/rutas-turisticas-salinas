#include "estructuras/ArregloDinamico.hpp"
#include "estructuras/ListaEnlazada.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}
template <typename Funcion>
void fueraDeRango(Funcion ejecutar) {
    try { ejecutar(); }
    catch (const std::out_of_range&) { return; }
    throw std::runtime_error("Se esperaba rechazar un indice invalido");
}

// Cuenta objetos vivos, incluidos los slots reservados del arreglo.
struct Recurso {
    inline static int vivos = 0;
    int valor = 0;
    Recurso() { ++vivos; }
    ~Recurso() { --vivos; }
    Recurso(const Recurso&) = delete;
    Recurso& operator=(const Recurso&) = delete;
    Recurso(Recurso&& otro) noexcept : valor(otro.valor) { ++vivos; otro.valor = 0; }
    Recurso& operator=(Recurso&& otro) noexcept {
        valor = otro.valor;
        otro.valor = 0;
        return *this;
    }
};

template <typename T>
void moverEntre(T& destino, T& origen) { destino = std::move(origen); }

void probarArreglo() {
    ArregloDinamico<int> numeros(0);
    comprobar(numeros.vacio() && numeros.capacidad() == 0, "Inicio vacio");
    fueraDeRango([&] { numeros.acceder(0); });
    fueraDeRango([&] { numeros.eliminar(0); });
    for (int i = 0; i < 1000; ++i) numeros.insertar(i);
    for (int i = 0; i < 1000; ++i) comprobar(numeros.acceder(i) == i, "Datos tras crecer");
    numeros.eliminar(0);
    numeros.eliminar(498);
    numeros.eliminar(numeros.cantidad() - 1);
    comprobar(numeros.cantidad() == 997 && numeros.acceder(498) == 500, "Desplazamiento al eliminar");
    const auto& consulta = numeros;
    comprobar(consulta.acceder(0) == 1, "Acceso const");
    fueraDeRango([&] { consulta.acceder(consulta.cantidad()); });

    ArregloDinamico<std::string> nombres(1);
    nombres.insertar("Salinas");
    nombres.insertar(nombres.acceder(0));
    comprobar(nombres.acceder(1) == "Salinas", "Insercion desde el mismo arreglo al crecer");

    ArregloDinamico<int> destino(std::move(numeros));
    comprobar(numeros.vacio() && destino.cantidad() == 997, "Constructor de movimiento");
    numeros.insertar(42);
    destino = std::move(numeros);
    comprobar(destino.cantidad() == 1 && destino.acceder(0) == 42, "Asignacion por movimiento");
    moverEntre(destino, destino);
    comprobar(destino.acceder(0) == 42, "Autoasignacion");
    destino.eliminar(0);
    comprobar(destino.vacio(), "Eliminar ultimo elemento");
}

void probarLista() {
    ListaEnlazada<int> lista;
    comprobar(lista.vacia(), "Lista inicialmente vacia");
    comprobar(lista.buscar([](int) { return true; }) == nullptr, "Busqueda en lista vacia");
    comprobar(lista.eliminarSi([](int) { return true; }) == 0, "Eliminar en lista vacia");
    for (int valor : {1, 2, 2, 3, 4}) lista.insertar(valor);
    comprobar(lista.eliminarSi([](int x) { return x == 2; }) == 2, "Coincidencias consecutivas");
    lista.eliminarSi([](int x) { return x == 1 || x == 4; });
    comprobar(lista.cantidad() == 1, "Eliminar cabeza y cola");
    lista.insertar(5);
    std::string orden;
    const auto& consulta = lista;
    consulta.recorrer([&](int x) { orden += std::to_string(x); });
    comprobar(orden == "35", "Orden de recorrido y cola tras eliminar");
    comprobar(*consulta.buscar([](int x) { return x == 5; }) == 5, "Busqueda const");
    *lista.buscar([](int x) { return x == 3; }) = 30;
    comprobar(*lista.buscar([](int x) { return x == 30; }) == 30, "Edicion de dato encontrado");
    ListaEnlazada<int> movida(std::move(lista));
    lista.insertar(8);
    movida = std::move(lista);
    comprobar(movida.cantidad() == 1 && lista.vacia(), "Mover lista y reemplazar nodos anteriores");
    moverEntre(movida, movida);
    movida.eliminarSi([](int) { return true; });
    movida.insertar(9);
    movida.limpiar();
    movida.limpiar();
    comprobar(movida.vacia(), "Limpieza repetida");
}

void probarPropiedad() {
    {
        // Simula el futuro arreglo de vertices, cada uno con conexiones propias.
        ArregloDinamico<ListaEnlazada<Recurso>> vertices(1);
        for (int i = 0; i < 100; ++i) {
            ListaEnlazada<Recurso> conexiones;
            Recurso recurso;
            recurso.valor = i;
            conexiones.insertar(std::move(recurso));
            vertices.insertar(std::move(conexiones));
        }
        comprobar(Recurso::vivos == 100, "Recursos tras redimensionar vertices");
        vertices.eliminar(50);
        comprobar(Recurso::vivos == 99, "Liberacion de conexiones del vertice eliminado");
        const auto* encontrado = vertices.acceder(50).buscar([](const Recurso& r) { return r.valor == 51; });
        comprobar(encontrado != nullptr, "Conexiones intactas tras desplazar vertices");
    }
    comprobar(Recurso::vivos == 0, "Todos los recursos deben liberarse al salir del bloque");
    {
        ArregloDinamico<Recurso> recursos;
        for (int i = 0; i < 100; ++i) recursos.insertar(Recurso{});
        ArregloDinamico<Recurso> otro;
        otro = std::move(recursos);
        otro.eliminar(0);
    }
    comprobar(Recurso::vivos == 0, "Liberacion del arreglo de recursos");
}

int main() {
    static_assert(!std::is_copy_constructible<ArregloDinamico<int>>::value);
    static_assert(!std::is_copy_constructible<ListaEnlazada<int>>::value);
    try {
        probarArreglo();
        std::cout << "OK: arreglo dinamico\n";
        probarLista();
        std::cout << "OK: lista enlazada\n";
        probarPropiedad();
        std::cout << "OK: propiedad de memoria y estructuras anidadas\n";
    } catch (const std::exception& error) {
        std::cerr << "FALLO: " << error.what() << '\n';
        return 1;
    }
}

