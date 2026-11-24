#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Almacenamiento propio: requiere T construible por defecto y movible sin excepciones.
// Las copias se deshabilitan: cada arreglo es dueno exclusivo de su memoria.
template <typename T>
class ArregloDinamico {
    static_assert(std::is_nothrow_move_assignable<T>::value,
                  "T debe permitir asignacion por movimiento sin excepciones");
    T* datos_ = nullptr;
    std::size_t cantidad_ = 0;
    std::size_t capacidad_ = 0;

    void reservar(std::size_t nuevaCapacidad) {
        if (nuevaCapacidad <= capacidad_) return;
        if (nuevaCapacidad > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::length_error("Capacidad demasiado grande");
        }
        // Si new falla, el arreglo original permanece intacto.
        T* nuevos = new T[nuevaCapacidad]{};
        for (std::size_t i = 0; i < cantidad_; ++i) {
            nuevos[i] = std::move(datos_[i]);
        }
        delete[] datos_;
        datos_ = nuevos;
        capacidad_ = nuevaCapacidad;
    }

public:
    explicit ArregloDinamico(std::size_t capacidadInicial = 4) {
        reservar(capacidadInicial);
    }
    ~ArregloDinamico() { delete[] datos_; }
    ArregloDinamico(const ArregloDinamico&) = delete;
    ArregloDinamico& operator=(const ArregloDinamico&) = delete;

    ArregloDinamico(ArregloDinamico&& otro) noexcept
        : datos_(otro.datos_), cantidad_(otro.cantidad_), capacidad_(otro.capacidad_) {
        otro.datos_ = nullptr;
        otro.cantidad_ = otro.capacidad_ = 0;
    }
    ArregloDinamico& operator=(ArregloDinamico&& otro) noexcept {
        if (this != &otro) {
            delete[] datos_;
            datos_ = otro.datos_;
            cantidad_ = otro.cantidad_;
            capacidad_ = otro.capacidad_;
            otro.datos_ = nullptr;
            otro.cantidad_ = otro.capacidad_ = 0;
        }
        return *this;
    }

    std::size_t cantidad() const noexcept { return cantidad_; }
    std::size_t capacidad() const noexcept { return capacidad_; }
    bool vacio() const noexcept { return cantidad_ == 0; }

    T& acceder(std::size_t indice) {
        if (indice >= cantidad_) throw std::out_of_range("Indice fuera del arreglo");
        return datos_[indice];
    }
    const T& acceder(std::size_t indice) const {
        if (indice >= cantidad_) throw std::out_of_range("Indice fuera del arreglo");
        return datos_[indice];
    }

    // Recibir por valor evita invalidar un argumento que provenga del propio arreglo.
    void insertar(T valor) {
        if (cantidad_ == capacidad_) {
            const auto limite = std::numeric_limits<std::size_t>::max() / sizeof(T);
            if (capacidad_ >= limite) throw std::length_error("Arreglo lleno");
            const auto siguiente = capacidad_ == 0 ? 1
                : (capacidad_ > limite / 2 ? limite : capacidad_ * 2);
            reservar(siguiente);
        }
        datos_[cantidad_] = std::move(valor);
        ++cantidad_;
    }

    void eliminar(std::size_t indice) {
        if (indice >= cantidad_) throw std::out_of_range("Indice fuera del arreglo");
        // Construir antes de modificar permite fallar sin alterar el arreglo.
        T vacio{};
        for (std::size_t i = indice; i + 1 < cantidad_; ++i) {
            datos_[i] = std::move(datos_[i + 1]);
        }
        datos_[cantidad_ - 1] = std::move(vacio);
        --cantidad_;
    }
};
