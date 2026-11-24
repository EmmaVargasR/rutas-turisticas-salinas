#pragma once
#include "httplib.h"
#include "../grafo/Grafo.hpp"
#include "../grafo/AlgoritmosRutas.hpp"
#include "../persistencia/PersistenciaMapa.hpp"
#include <charconv>
#include <iomanip>
#include <locale>
#include <mutex>
#include <sstream>

namespace api {
inline std::string textoJson(const std::string& texto) {
    std::string resultado = "\"";
    const char* hex = "0123456789abcdef";
    for (unsigned char c : texto) {
        if (c == '"' || c == '\\') { resultado += '\\'; resultado += static_cast<char>(c); }
        else if (c < 32) {
            resultado += "\\u00";
            resultado += hex[c >> 4];
            resultado += hex[c & 15];
        } else resultado += static_cast<char>(c);
    }
    return resultado + '"';
}
inline void responder(httplib::Response& res, const std::string& contenido, int estado = 200) {
    res.status = estado;
    res.set_content(contenido, "application/json; charset=utf-8");
}
inline std::string campo(const httplib::Request& req, const std::string& nombre, std::size_t maximo = 200) {
    if (req.get_param_value_count(nombre) != 1) throw std::invalid_argument("Falta o se repite el campo: " + nombre);
    auto valor = req.get_param_value(nombre);
    if (valor.size() > maximo) throw std::invalid_argument("El campo es demasiado largo: " + nombre);
    return valor;
}
inline int entero(const std::string& texto) {
    int valor = 0;
    const auto resultado = std::from_chars(texto.data(), texto.data() + texto.size(), valor);
    if (resultado.ec != std::errc{} || resultado.ptr != texto.data() + texto.size() || valor <= 0) {
        throw std::invalid_argument("Se esperaba un ID entero positivo");
    }
    return valor;
}
inline double numero(const std::string& texto) {
    double valor = 0;
    std::istringstream entrada(texto);
    entrada.imbue(std::locale::classic());
    entrada >> std::noskipws >> valor;
    if (!entrada || !entrada.eof() || !std::isfinite(valor) || valor <= 0) {
        throw std::invalid_argument("Distancia y tiempo deben ser numeros positivos y finitos");
    }
    return valor;
}
inline bool booleano(const httplib::Request& req, const std::string& nombre) {
    const auto valor = campo(req, nombre);
    if (valor == "true") return true;
    if (valor == "false") return false;
    throw std::invalid_argument("Valor de estado incorrecto: " + nombre);
}
inline Lugar leerLugar(const httplib::Request& req, int id) {
    return {id, campo(req, "nombre"), campo(req, "descripcion", 4000),
            campo(req, "categoria", 100), booleano(req, "activo")};
}
inline std::string lugaresJson(const Grafo& grafo) {
    std::string salida = "[";
    for (std::size_t i = 0; i < grafo.cantidadLugares(); ++i) {
        if (i) salida += ',';
        const auto& l = grafo.verticeEn(i).lugar;
        salida += "{\"id\":" + std::to_string(l.id) + ",\"nombre\":" + textoJson(l.nombre)
            + ",\"descripcion\":" + textoJson(l.descripcion) + ",\"categoria\":" + textoJson(l.categoria)
            + ",\"activo\":" + (l.activo ? "true" : "false") + "}";
    }
    return salida + ']';
}
inline std::string caminosJson(const Grafo& grafo) {
    std::ostringstream salida;
    salida.imbue(std::locale::classic());
    salida << std::setprecision(17) << '[';
    bool primero = true;
    for (std::size_t i = 0; i < grafo.cantidadLugares(); ++i) {
        const auto& v = grafo.verticeEn(i);
        v.conexiones.recorrer([&](const Camino& c) {
            if (c.bidireccional && v.lugar.id > c.destino) return;
            if (!primero) salida << ',';
            primero = false;
            salida << "{\"id\":" << c.id << ",\"origen\":" << v.lugar.id
                   << ",\"destino\":" << c.destino << ",\"distancia\":" << c.distanciaMetros
                   << ",\"tiempo\":" << c.tiempoMinutos
                   << ",\"bidireccional\":" << (c.bidireccional ? "true" : "false")
                   << ",\"habilitado\":" << (c.habilitado ? "true" : "false")
                   << ",\"nota\":" << textoJson(c.nota) << '}';
        });
    }
    salida << ']';
    return salida.str();
}

inline std::string rutaJson(const Grafo& grafo, const ResultadoRuta& ruta) {
    std::ostringstream salida;
    salida.imbue(std::locale::classic());
    salida << std::setprecision(17) << "{\"encontrada\":" << (ruta.encontrada ? "true" : "false")
           << ",\"criterio\":\"" << (ruta.criterio == CriterioRuta::Distancia ? "distancia" : "tiempo")
           << "\",\"distanciaTotal\":" << ruta.distanciaTotal << ",\"tiempoTotal\":" << ruta.tiempoTotal
           << ",\"lugares\":[";
    for (std::size_t i = 0; i < ruta.lugares.cantidad(); ++i) {
        if (i) salida << ',';
        const auto* lugar = grafo.buscarLugar(ruta.lugares.acceder(i));
        salida << "{\"id\":" << lugar->id << ",\"nombre\":" << textoJson(lugar->nombre) << '}';
    }
    salida << "],\"tramos\":[";
    for (std::size_t i = 0; i < ruta.tramos.cantidad(); ++i) {
        if (i) salida << ',';
        const auto& t = ruta.tramos.acceder(i);
        salida << "{\"idCamino\":" << t.idCamino << ",\"origen\":" << t.origen << ",\"destino\":" << t.destino
               << ",\"distancia\":" << t.distanciaMetros << ",\"tiempo\":" << t.tiempoMinutos
               << ",\"nota\":" << textoJson(grafo.buscarCamino(t.idCamino)->nota) << '}';
    }
    salida << "]}";
    return salida.str();
}

inline void registrarRutas(httplib::Server& servidor, Grafo& grafo, std::mutex& candado, PersistenciaMapa& archivo) {
    // El servidor usa varios hilos; el bloqueo protege consultas y modificaciones.
    auto proteger = [&candado, &grafo, &archivo](auto accion) -> httplib::Server::Handler {
        return [&candado, &grafo, &archivo, accion](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> bloqueo(candado);
            try {
                if (req.method != "GET") {
                    if (req.get_header_value("X-Salinas-Client") != "web") {
                        responder(res, "{\"error\":\"Solicitud no autorizada\"}", 403);
                        return;
                    }
                    if (req.method != "DELETE" && req.get_header_value("Content-Type").find(
                        "application/x-www-form-urlencoded") != 0) {
                        responder(res, "{\"error\":\"Se esperaba un formulario\"}", 415);
                        return;
                    }
                }
                const bool cambio = req.method != "GET" && req.path != "/api/guardar" && req.path != "/api/cargar";
                if (cambio) {
                    auto anterior = PersistenciaMapa::copiar(grafo);
                    try {
                        accion(req, res);
                        archivo.guardar(grafo);
                    } catch (...) {
                        grafo = std::move(anterior);
                        throw;
                    }
                } else accion(req, res);
            } catch (const std::invalid_argument& e) {
                responder(res, "{\"error\":" + textoJson(e.what()) + "}", 400);
            } catch (const std::overflow_error& e) {
                responder(res, "{\"error\":" + textoJson(e.what()) + "}", 400);
            } catch (const std::out_of_range& e) {
                responder(res, "{\"error\":" + textoJson(e.what()) + "}", 404);
            } catch (const ErrorPersistencia& e) {
                responder(res, "{\"error\":" + textoJson(e.what()) + "}", 500);
            } catch (const std::filesystem::filesystem_error&) {
                responder(res, "{\"error\":\"No se pudo acceder al archivo; se conserva el mapa anterior\"}", 500);
            } catch (const std::exception&) {
                responder(res, "{\"error\":\"No se pudo completar la operacion\"}", 500);
            }
        };
    };
    servidor.Post("/api/guardar", proteger([&grafo, &archivo](const auto&, auto& res) {
        archivo.guardar(grafo);
        responder(res, "{\"mensaje\":\"Mapa guardado en esta computadora\"}");
    }));
    servidor.Post("/api/cargar", proteger([&grafo, &archivo](const auto&, auto& res) {
        if (!archivo.cargar(grafo)) throw std::out_of_range("Todavia no existe un archivo guardado");
        responder(res, "{\"mensaje\":\"Mapa cargado y validado correctamente\"}");
    }));
    servidor.Get("/api/ruta", proteger([&grafo](const auto& req, auto& res) {
        const auto criterio = campo(req, "criterio");
        if (criterio != "distancia" && criterio != "tiempo") {
            throw std::invalid_argument("El criterio debe ser distancia o tiempo");
        }
        const auto ruta = AlgoritmosRutas::dijkstra(grafo, entero(campo(req, "origen")),
            entero(campo(req, "destino")), criterio == "distancia" ? CriterioRuta::Distancia : CriterioRuta::Tiempo);
        responder(res, rutaJson(grafo, ruta));
    }));
    servidor.Get("/api/mapa", proteger([&grafo](const auto&, auto& res) {
        responder(res, "{\"lugares\":" + lugaresJson(grafo) + ",\"caminos\":" + caminosJson(grafo)
            + ",\"totalArcos\":" + std::to_string(grafo.cantidadArcos()) + ",\"persistencia\":true}");
    }));
    servidor.Get("/api/lugares", proteger([&grafo](const auto&, auto& res) {
        responder(res, "{\"lugares\":" + lugaresJson(grafo) + "}");
    }));
    servidor.Get("/api/caminos", proteger([&grafo](const auto&, auto& res) {
        responder(res, "{\"caminos\":" + caminosJson(grafo) + "}");
    }));
    servidor.Post("/api/lugares", proteger([&grafo](const auto& req, auto& res) {
        grafo.registrarLugar(leerLugar(req, entero(campo(req, "id"))));
        responder(res, "{\"mensaje\":\"Lugar registrado\"}", 201);
    }));
    servidor.Put(R"(/api/lugares/([0-9]+))", proteger([&grafo](const auto& req, auto& res) {
        grafo.editarLugar(leerLugar(req, entero(req.matches[1].str())));
        responder(res, "{\"mensaje\":\"Lugar actualizado\"}");
    }));
    servidor.Delete(R"(/api/lugares/([0-9]+))", proteger([&grafo](const auto& req, auto& res) {
        grafo.eliminarLugar(entero(req.matches[1].str()));
        responder(res, "{\"mensaje\":\"Lugar y conexiones asociadas eliminados\"}");
    }));
    servidor.Post("/api/caminos", proteger([&grafo](const auto& req, auto& res) {
        grafo.conectar(entero(campo(req, "id")), entero(campo(req, "origen")), entero(campo(req, "destino")),
            numero(campo(req, "distancia")), numero(campo(req, "tiempo")), booleano(req, "bidireccional"),
            booleano(req, "habilitado"), req.has_param("nota") ? campo(req, "nota", 2000) : "");
        responder(res, "{\"mensaje\":\"Camino registrado\"}", 201);
    }));
    servidor.Put(R"(/api/caminos/([0-9]+))", proteger([&grafo](const auto& req, auto& res) {
        grafo.editarCamino(entero(req.matches[1].str()), numero(campo(req, "distancia")),
            numero(campo(req, "tiempo")), booleano(req, "habilitado"),
            req.has_param("nota") ? std::optional<std::string>(campo(req, "nota", 2000)) : std::nullopt);
        responder(res, "{\"mensaje\":\"Camino actualizado en todos sus sentidos\"}");
    }));
    servidor.Delete(R"(/api/caminos/([0-9]+))", proteger([&grafo](const auto& req, auto& res) {
        grafo.eliminarCamino(entero(req.matches[1].str()));
        responder(res, "{\"mensaje\":\"Camino eliminado\"}");
    }));
}
} // namespace api
