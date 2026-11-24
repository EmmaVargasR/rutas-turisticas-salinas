#include "persistencia/PersistenciaMapa.hpp"
#include "grafo/AlgoritmosRutas.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}
template <typename Error, typename F>
void rechazar(F ejecutar) {
    try { ejecutar(); } catch (const Error&) { return; }
    throw std::runtime_error("Se esperaba rechazar el archivo o la operacion");
}
void escribir(const fs::path& ruta, const std::string& valor) {
    std::ofstream salida(ruta, std::ios::binary | std::ios::trunc);
    salida << valor;
    if (!salida) throw std::runtime_error("No se pudo preparar la prueba");
}
std::string leerTexto(const fs::path& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    std::ostringstream texto;
    texto << entrada.rdbuf();
    return texto.str();
}
Lugar lugar(int id) {
    return {id, "Punto \"Arbol\" " + std::to_string(id), "Linea 1\nLinea 2\t\\ ruta & texto", "Prueba", true};
}
struct CarpetaTemporal {
    fs::path ruta;
    CarpetaTemporal() {
        const auto base = fs::absolute("build").lexically_normal();
        ruta = base / ("persistencia-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        comprobar(ruta.parent_path() == base, "Carpeta de prueba fuera de build");
        fs::create_directories(ruta);
    }
    ~CarpetaTemporal() {
        const auto base = fs::absolute("build").lexically_normal();
        std::error_code error;
        if (ruta.lexically_normal().parent_path() == base) fs::remove_all(ruta, error);
    }
};
void probarArchivos(const fs::path& carpeta) {
    const auto ruta = carpeta / "mapa.txt";
    PersistenciaMapa archivo(ruta);
    Grafo g;
    g.registrarLugar(lugar(99));
    comprobar(!archivo.cargar(g) && g.buscarLugar(99), "Archivo ausente conserva el grafo");
    Grafo original;
    for (int id : {10, 20, 30}) original.registrarLugar(lugar(id));
    auto inactivo = lugar(30); inactivo.activo = false; original.editarLugar(inactivo);
    const std::string nota = "Aproximado: \"mapa\" \\ revisar\nEscaleras";
    original.conectar(1, 10, 20, 123.456789012345, 2.3456789012345, true, true, nota);
    original.conectar(2, 20, 30, 20, 1, false, false);
    archivo.guardar(original);
    comprobar(archivo.cargar(g), "Carga del archivo creado");
    comprobar(g.cantidadLugares() == 3 && g.cantidadCaminos() == 2 && g.cantidadArcos() == 3, "Cantidades y sentidos");
    comprobar(g.buscarLugar(10)->nombre == lugar(10).nombre && g.buscarLugar(10)->descripcion == lugar(10).descripcion, "Comillas, barras y saltos de linea");
    comprobar(!g.buscarLugar(30)->activo && !g.consultarConexion(20, 30)->habilitado, "Estados preservados");
    comprobar(g.consultarConexion(10, 20)->nota == nota && g.consultarConexion(20, 10)->nota == nota, "Nota persistente en ambos sentidos");
    original.editarCamino(1, 123.456789012345, 2.3456789012345, true);
    comprobar(original.buscarCamino(1)->nota == nota, "Editar pesos sin nota conserva su procedencia");
    comprobar(g.consultarConexion(10, 20)->distanciaMetros == original.consultarConexion(10, 20)->distanciaMetros, "Precision decimal");
    comprobar(AlgoritmosRutas::dijkstra(g, 20, 10, CriterioRuta::Distancia).encontrada, "Dijkstra tras cargar");
    const auto primeraVersion = leerTexto(ruta);
    original.eliminarLugar(30);
    archivo.guardar(original);
    comprobar(leerTexto(ruta.string() + ".bak") == primeraVersion, "Respaldo de la version anterior");
    Grafo copia = PersistenciaMapa::copiar(original);
    comprobar(copia.buscarCamino(1)->nota == nota, "Transaccion conserva la nota");
    copia.eliminarLugar(10);
    comprobar(original.buscarLugar(10) && !copia.buscarLugar(10), "Copia de transaccion independiente");
    const auto valido = leerTexto(ruta);
    const auto respaldo = leerTexto(ruta.string() + ".bak");
    const std::string cabecera = "SALINAS_MAPA 1\nLUGARES 1\nL 1 1 \"Uno\" \"\" \"Prueba\"\n";
    for (const auto& invalido : {
        std::string(""), std::string("SALINAS_MAPA 99\nLUGARES 0\nCAMINOS 0\nFIN\n"),
        std::string("SALINAS_MAPA 1\nLUGARES 1001\n"),
        cabecera + "CAMINOS 1\nC 1 1 2 10 1 0 1\nFIN\n",
        cabecera + "CAMINOS 1\nC 1 1 1 10 1 0 1\nFIN\n",
        cabecera + "CAMINOS 0\n", cabecera + "CAMINOS 0\nFIN\nBASURA",
        std::string("SALINAS_MAPA 1\nLUGARES 1\nL 1 2 \"Uno\" \"\" \"Prueba\"\nCAMINOS 0\nFIN\n"),
        std::string("SALINAS_MAPA 1\nLUGARES 2\nL 1 1 \"Uno\" \"\" \"Prueba\"\nL 1 1 \"Dos\" \"\" \"Prueba\"\nCAMINOS 0\nFIN\n"),
        std::string("SALINAS_MAPA 1\nLUGARES 2\nL 1 1 \"Uno\" \"\" \"Prueba\"\nL 2 1 \"Dos\" \"\" \"Prueba\"\nCAMINOS 1\nC 1 1 2 -10 1 0 1\nFIN\n")
    }) {
        escribir(ruta, invalido);
        rechazar<std::invalid_argument>([&] { archivo.cargar(g); });
        comprobar(g.cantidadLugares() == 3 && g.buscarLugar(30), "Carga fallida conserva todo el grafo");
        rechazar<std::invalid_argument>([&] { archivo.guardar(original); });
        comprobar(leerTexto(ruta) == invalido && leerTexto(ruta.string() + ".bak") == respaldo, "Archivo invalido y respaldo intactos");
    }
    escribir(ruta, valido);
    const auto temporal = fs::path(ruta.string() + ".tmp");
    fs::create_directory(temporal); // Fallo de escritura reproducible, sin cambiar permisos.
    rechazar<ErrorPersistencia>([&] { archivo.guardar(original); });
    comprobar(leerTexto(ruta) == valido && fs::is_directory(temporal), "Fallo al guardar conserva archivo y directorio ajeno");
    fs::remove(temporal);
    rechazar<ErrorPersistencia>([&] { PersistenciaMapa otro(ruta); });
    Grafo vacio;
    archivo.guardar(vacio);
    comprobar(archivo.cargar(g) && g.cantidadLugares() == 0, "Guardar eliminacion de todos los lugares");
    escribir(ruta, "SALINAS_MAPA 1\nLUGARES 2\nL 1 1 \"Uno\" \"\" \"Prueba\"\nL 2 1 \"Dos\" \"\" \"Prueba\"\nCAMINOS 1\nC 1 1 2 10 1 1 1\nFIN\n");
    comprobar(archivo.cargar(g) && g.buscarCamino(1)->nota.empty(), "Compatibilidad con archivos previos sin notas");
    escribir(ruta, "SALINAS_MAPA 2\nLUGARES 2\nL 1 1 \"Uno\" \"\" \"Prueba\"\nL 2 1 \"Dos\" \"\" \"Prueba\"\nCAMINOS 1\nC 1 1 2 10 1 1 1\nFIN\n");
    rechazar<std::invalid_argument>([&] { archivo.cargar(g); });
}
int main() {
    try {
        CarpetaTemporal carpeta;
        probarArchivos(carpeta.ruta);
        std::cout << "OK: guardar/cargar, estados, precision, respaldo y copia independiente\n";
        std::cout << "OK: archivos invalidos y fallos de escritura conservan los datos\n";
        std::cout << "OK: bloqueo de doble apertura y mapa vacio persistente\n";
    } catch (const std::exception& e) {
        std::cerr << "FALLO: " << e.what() << '\n';
        return 1;
    }
}
