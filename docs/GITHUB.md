# Repositorio del proyecto

Repositorio: [EmmaVargasR/rutas-turisticas-salinas](https://github.com/EmmaVargasR/rutas-turisticas-salinas).

La versión anterior se llamaba Salinas-Viva y correspondía a otra idea basada en Unity. Se conservó su historial y una rama con el estado anterior: [codex/respaldo-salinas-viva-2026-11-23](https://github.com/EmmaVargasR/rutas-turisticas-salinas/tree/codex/respaldo-salinas-viva-2026-11-23). La rama principal contiene el sistema actual con motor C++ y web local.

## Descargar en otra computadora

```powershell
git clone https://github.com/EmmaVargasR/rutas-turisticas-salinas.git
cd rutas-turisticas-salinas
.\iniciar.ps1
```

También se puede descargar el ZIP desde Code → Download ZIP. Se necesita un compilador C++17 compatible, como el MinGW de CodeBlocks; consulta el README principal. La aplicación se abre en http://127.0.0.1:8080.

El primer arranque utiliza `data/mapa-inicial.txt`, con cinco lugares y tres caminos aproximados. No reemplaza un `data/mapa.txt` existente. Para trasladar cambios posteriores del catálogo a otra computadora, copia además ese archivo de trabajo con el servidor detenido; no se incluye automáticamente en los commits.

## Guardar cambios posteriores

Desde la carpeta del proyecto, con GitHub autenticado:

```powershell
git status
git add .
git commit -m "Describe el cambio realizado"
git push
```

Antes de confirmar, revisa los archivos mostrados por `git status`. Los ejecutables, temporales, respaldos, archivos de trabajo y cachés de Unity se excluyen mediante `.gitignore`.

El avance de Unity se conserva en `unity/` como extra pendiente de comprobar; no es necesario para ejecutar la aplicación principal. GitHub almacena el código y la documentación: la aplicación con servidor C++ continúa ejecutándose localmente.
