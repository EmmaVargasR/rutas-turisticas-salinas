#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif
#include "../grafo/Grafo.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <system_error>

class ErrorPersistencia : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class PersistenciaMapa {
    std::filesystem::path archivo_;
#ifdef _WIN32
    HANDLE bloqueo_ = INVALID_HANDLE_VALUE;
#else
    int bloqueo_ = -1;
#endif
    static constexpr std::uintmax_t maximoBytes = 8 * 1024 * 1024;

    static void exigir(bool condicion) {
        if (!condicion) throw std::invalid_argument("Archivo de mapa invalido o incompleto; no se cargaron cambios");
    }
    static void etiqueta(std::istream& entrada, const char* esperada) {
        std::string leida;
        exigir(static_cast<bool>(entrada >> leida) && leida == esperada);
    }
    static std::string texto(std::istream& entrada, std::size_t maximo) {
        entrada >> std::ws;
        exigir(entrada.peek() == '"');
        std::string valor;
        exigir(static_cast<bool>(entrada >> std::quoted(valor)) && valor.size() <= maximo);
        return valor;
    }
    static Grafo leer(const std::filesystem::path& ruta) {
        if (!std::filesystem::is_regular_file(ruta)) throw ErrorPersistencia("El archivo del mapa no es un archivo de texto accesible");
        exigir(std::filesystem::file_size(ruta) <= maximoBytes);
        std::ifstream entrada(ruta, std::ios::binary);
        if (!entrada) throw ErrorPersistencia("No se pudo abrir el archivo del mapa");
        entrada.imbue(std::locale::classic());
        etiqueta(entrada, "SALINAS_MAPA");
        int version = 0;
        exigir(static_cast<bool>(entrada >> version) && (version == 1 || version == 2));
        etiqueta(entrada, "LUGARES");
        std::size_t cantidad = 0;
        exigir(static_cast<bool>(entrada >> cantidad) && cantidad <= 1000);
        Grafo candidato;
        try {
            for (std::size_t i = 0; i < cantidad; ++i) {
                etiqueta(entrada, "L");
                Lugar lugar;
                int activo = -1;
                exigir(static_cast<bool>(entrada >> lugar.id >> activo) && (activo == 0 || activo == 1));
                lugar.activo = activo == 1;
                lugar.nombre = texto(entrada, 200);
                lugar.descripcion = texto(entrada, 4000);
                lugar.categoria = texto(entrada, 100);
                candidato.registrarLugar(std::move(lugar));
            }
            etiqueta(entrada, "CAMINOS");
            exigir(static_cast<bool>(entrada >> cantidad) && cantidad <= 10000);
            for (std::size_t i = 0; i < cantidad; ++i) {
                etiqueta(entrada, "C");
                int id = 0, origen = 0, destino = 0, doble = -1, habilitado = -1;
                double distancia = 0, tiempo = 0;
                exigir(static_cast<bool>(entrada >> id >> origen >> destino >> distancia >> tiempo >> doble >> habilitado)
                    && (doble == 0 || doble == 1) && (habilitado == 0 || habilitado == 1));
                const auto nota = version >= 2 ? texto(entrada, 2000) : std::string{};
                candidato.conectar(id, origen, destino, distancia, tiempo, doble == 1, habilitado == 1, nota);
            }
        } catch (const std::out_of_range&) {
            throw std::invalid_argument("Archivo invalido: un camino referencia un lugar inexistente");
        }
        etiqueta(entrada, "FIN");
        entrada >> std::ws;
        exigir(entrada.eof() && !entrada.bad());
        return candidato;
    }
    static void reemplazar(const std::filesystem::path& origen, const std::filesystem::path& destino) {
#ifdef _WIN32
        if (!MoveFileExW(origen.c_str(), destino.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            throw ErrorPersistencia("No se pudo reemplazar el archivo; se conserva la version anterior");
        }
#else
        std::error_code error;
        std::filesystem::rename(origen, destino, error);
        if (error) throw ErrorPersistencia("No se pudo reemplazar el archivo; se conserva la version anterior");
#endif
    }
    static void limpiarTemporal(const std::filesystem::path& ruta) noexcept {
        std::error_code error;
        if (std::filesystem::is_regular_file(ruta, error)) std::filesystem::remove(ruta, error);
    }

public:
    explicit PersistenciaMapa(std::filesystem::path archivo) : archivo_(std::move(archivo)) {
        if (archivo_.empty() || archivo_.filename().empty()) throw ErrorPersistencia("Ruta del mapa no valida");
        if (!archivo_.parent_path().empty()) std::filesystem::create_directories(archivo_.parent_path());
        const auto rutaBloqueo = std::filesystem::path(archivo_.native() + std::filesystem::path(".lock").native());
#ifdef _WIN32
        bloqueo_ = CreateFileW(rutaBloqueo.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (bloqueo_ == INVALID_HANDLE_VALUE) throw ErrorPersistencia("No se pudo abrir el mapa: ya esta en uso o no hay permisos de escritura");
#else
        bloqueo_ = ::open(rutaBloqueo.c_str(), O_CREAT | O_RDWR, 0600);
        if (bloqueo_ < 0) throw ErrorPersistencia("No se pudo abrir el archivo de bloqueo");
        if (::flock(bloqueo_, LOCK_EX | LOCK_NB) != 0) {
            ::close(bloqueo_);
            bloqueo_ = -1;
            throw ErrorPersistencia("El mapa ya esta en uso por otro servidor");
        }
#endif
    }
    ~PersistenciaMapa() {
#ifdef _WIN32
        if (bloqueo_ != INVALID_HANDLE_VALUE) CloseHandle(bloqueo_);
#else
        if (bloqueo_ >= 0) ::close(bloqueo_);
#endif
    }
    PersistenciaMapa(const PersistenciaMapa&) = delete;
    PersistenciaMapa& operator=(const PersistenciaMapa&) = delete;

    static Grafo copiar(const Grafo& original) {
        Grafo copia;
        for (std::size_t i = 0; i < original.cantidadLugares(); ++i) copia.registrarLugar(original.verticeEn(i).lugar);
        for (std::size_t i = 0; i < original.cantidadLugares(); ++i) {
            const auto& v = original.verticeEn(i);
            v.conexiones.recorrer([&](const Camino& c) {
                if (!c.bidireccional || v.lugar.id < c.destino) {
                    copia.conectar(c.id, v.lugar.id, c.destino, c.distanciaMetros, c.tiempoMinutos, c.bidireccional, c.habilitado, c.nota);
                }
            });
        }
        return copia;
    }
    // Construir y validar otro grafo antes de reemplazar el actual.
    bool cargar(Grafo& actual) const {
        if (!std::filesystem::exists(archivo_)) return false;
        auto candidato = leer(archivo_);
        actual = std::move(candidato);
        return true;
    }
    void guardar(const Grafo& grafo) const {
        const auto temporal = std::filesystem::path(archivo_.native() + std::filesystem::path(".tmp").native());
        const auto respaldo = std::filesystem::path(archivo_.native() + std::filesystem::path(".bak").native());
        const auto temporalRespaldo = std::filesystem::path(archivo_.native() + std::filesystem::path(".bak.tmp").native());
        try {
            // No sobrescribir silenciosamente un archivo alterado o corrupto.
            const bool existia = std::filesystem::exists(archivo_);
            if (existia) { auto validado = leer(archivo_); (void)validado; }
            if (grafo.cantidadLugares() > 1000 || grafo.cantidadCaminos() > 10000) {
                throw std::invalid_argument("El mapa supera los limites del formato de archivo");
            }
            std::ofstream salida(temporal, std::ios::binary | std::ios::trunc);
            if (!salida) throw ErrorPersistencia("No se pudo guardar el mapa; no se aplicaron cambios");
            salida.imbue(std::locale::classic());
            salida << std::setprecision(17) << "SALINAS_MAPA 2\nLUGARES " << grafo.cantidadLugares() << '\n';
            for (std::size_t i = 0; i < grafo.cantidadLugares(); ++i) {
                const auto& l = grafo.verticeEn(i).lugar;
                salida << "L " << l.id << ' ' << (l.activo ? 1 : 0) << ' ' << std::quoted(l.nombre) << ' '
                       << std::quoted(l.descripcion) << ' ' << std::quoted(l.categoria) << '\n';
            }
            salida << "CAMINOS " << grafo.cantidadCaminos() << '\n';
            for (std::size_t i = 0; i < grafo.cantidadLugares(); ++i) {
                const auto& v = grafo.verticeEn(i);
                v.conexiones.recorrer([&](const Camino& c) {
                    if (!c.bidireccional || v.lugar.id < c.destino) {
                        salida << "C " << c.id << ' ' << v.lugar.id << ' ' << c.destino << ' ' << c.distanciaMetros
                               << ' ' << c.tiempoMinutos << ' ' << (c.bidireccional ? 1 : 0) << ' '
                               << (c.habilitado ? 1 : 0) << ' ' << std::quoted(c.nota) << '\n';
                    }
                });
            }
            salida << "FIN\n";
            salida.close();
            if (!salida) throw ErrorPersistencia("Fallo al escribir el mapa; se conserva el archivo anterior");
            auto comprobado = leer(temporal); // Releer antes de reemplazar el archivo valido.
            (void)comprobado;
            if (existia) {
                std::filesystem::copy_file(archivo_, temporalRespaldo, std::filesystem::copy_options::overwrite_existing);
                reemplazar(temporalRespaldo, respaldo);
            }
            reemplazar(temporal, archivo_);
        } catch (...) {
            limpiarTemporal(temporal);
            limpiarTemporal(temporalRespaldo);
            throw;
        }
    }
};
