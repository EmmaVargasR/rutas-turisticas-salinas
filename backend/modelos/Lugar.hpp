#pragma once
#include <string>

struct Lugar {
    int id = 0;
    std::string nombre;
    std::string descripcion;
    std::string categoria;
    bool activo = true;
};
