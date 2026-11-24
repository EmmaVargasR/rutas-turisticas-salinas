#include "httplib.h"
#include "api/ApiGrafo.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc > 3) throw std::invalid_argument("Uso: salinas.exe [puerto] [archivo del mapa]");
        const int puerto = argc >= 2 ? api::entero(argv[1]) : 8080;
        if (puerto > 65535) throw std::invalid_argument("Puerto fuera de rango");
        Grafo grafo;
        PersistenciaMapa archivo(argc == 3 ? argv[2] : "data/mapa.txt");
        const bool recuperado = archivo.cargar(grafo);
        std::mutex candado;
        httplib::Server servidor;
        servidor.set_payload_max_length(16384);
        servidor.set_default_headers({{"Cache-Control", "no-store"}});
        api::registrarRutas(servidor, grafo, candado, archivo);
        servidor.Get("/api/estado", [](const httplib::Request&, httplib::Response& respuesta) {
            api::responder(respuesta,
                R"({"estado":"ok","motor":"C++17","etapa":"Rutas y persistencia","persistencia":true,"guardadoAutomatico":true})");
        });
        servidor.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            if (req.path.find("/api/") == 0 && res.body.empty()) {
                api::responder(res, "{\"error\":\"Solicitud no disponible o demasiado grande\"}", res.status);
            }
        });
        if (!servidor.set_mount_point("/", "./frontend")) throw std::runtime_error("No se encontro frontend. Ejecuta iniciar.ps1 desde el proyecto.");
        std::cout << (recuperado ? "Mapa recuperado desde el archivo.\n" : "Sin archivo previo. El primer cambio se guardara automaticamente.\n")
                  << "Salinas | Abre http://127.0.0.1:" << puerto << '\n'
                  << "Guardado automatico activo. Pulsa Ctrl+C para detener el servidor.\n" << std::flush;
        if (!servidor.listen("127.0.0.1", puerto)) throw std::runtime_error("No se pudo abrir el puerto del servidor");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "No se pudo iniciar Salinas: " << error.what()
                  << "\nNo se reemplazo el archivo guardado.\n";
        return 1;
    }
}
