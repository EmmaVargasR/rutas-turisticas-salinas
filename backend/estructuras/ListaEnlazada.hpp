#pragma once

#include <cstddef>
#include <utility>

// Lista simple propia para las conexiones salientes de cada vertice.
template <typename T>
class ListaEnlazada {
    struct Nodo {
        T valor;
        Nodo* siguiente = nullptr;
        explicit Nodo(T dato) : valor(std::move(dato)) {}
    };
    Nodo* primero_ = nullptr;
    Nodo* ultimo_ = nullptr;
    std::size_t cantidad_ = 0;

public:
    ListaEnlazada() = default;
    ~ListaEnlazada() { limpiar(); }
    ListaEnlazada(const ListaEnlazada&) = delete;
    ListaEnlazada& operator=(const ListaEnlazada&) = delete;

    ListaEnlazada(ListaEnlazada&& otra) noexcept
        : primero_(otra.primero_), ultimo_(otra.ultimo_), cantidad_(otra.cantidad_) {
        otra.primero_ = otra.ultimo_ = nullptr;
        otra.cantidad_ = 0;
    }
    ListaEnlazada& operator=(ListaEnlazada&& otra) noexcept {
        if (this != &otra) {
            limpiar();
            primero_ = otra.primero_;
            ultimo_ = otra.ultimo_;
            cantidad_ = otra.cantidad_;
            otra.primero_ = otra.ultimo_ = nullptr;
            otra.cantidad_ = 0;
        }
        return *this;
    }

    bool vacia() const noexcept { return cantidad_ == 0; }
    std::size_t cantidad() const noexcept { return cantidad_; }

    void insertar(T valor) {
        Nodo* nuevo = new Nodo(std::move(valor));
        if (ultimo_) ultimo_->siguiente = nuevo;
        else primero_ = nuevo;
        ultimo_ = nuevo;
        ++cantidad_;
    }

    template <typename Predicado>
    T* buscar(Predicado coincide) {
        for (Nodo* actual = primero_; actual; actual = actual->siguiente) {
            if (coincide(static_cast<const T&>(actual->valor))) return &actual->valor;
        }
        return nullptr;
    }
    template <typename Predicado>
    const T* buscar(Predicado coincide) const {
        for (const Nodo* actual = primero_; actual; actual = actual->siguiente) {
            if (coincide(actual->valor)) return &actual->valor;
        }
        return nullptr;
    }

    // Elimina todas las coincidencias: servira para limpiar conexiones a un lugar.
    template <typename Predicado>
    std::size_t eliminarSi(Predicado coincide) {
        std::size_t eliminados = 0;
        Nodo* anterior = nullptr;
        Nodo* actual = primero_;
        while (actual) {
            if (coincide(static_cast<const T&>(actual->valor))) {
                Nodo* siguiente = actual->siguiente;
                if (anterior) anterior->siguiente = siguiente;
                else primero_ = siguiente;
                if (ultimo_ == actual) ultimo_ = anterior;
                delete actual;
                actual = siguiente;
                --cantidad_;
                ++eliminados;
            } else {
                anterior = actual;
                actual = actual->siguiente;
            }
        }
        return eliminados;
    }

    template <typename Funcion>
    void recorrer(Funcion visitar) {
        for (Nodo* actual = primero_; actual; actual = actual->siguiente) visitar(actual->valor);
    }
    template <typename Funcion>
    void recorrer(Funcion visitar) const {
        for (const Nodo* actual = primero_; actual; actual = actual->siguiente) visitar(actual->valor);
    }

    void limpiar() noexcept {
        while (primero_) {
            Nodo* siguiente = primero_->siguiente;
            delete primero_;
            primero_ = siguiente;
        }
        ultimo_ = nullptr;
        cantidad_ = 0;
    }
};
