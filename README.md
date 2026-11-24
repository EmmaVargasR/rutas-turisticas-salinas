# Salinas · Caminos por descubrir

Proyecto de Programación Avanzada: interfaz web local y motor C++17.
Entrega: 27 de noviembre de 2026. Calendario y alcance en [PLAN.md](PLAN.md).

Repositorio: [EmmaVargasR/rutas-turisticas-salinas](https://github.com/EmmaVargasR/rutas-turisticas-salinas). Descarga, respaldo de la idea anterior y guardado de cambios en [docs/GITHUB.md](docs/GITHUB.md).

## Descargar y comenzar

Descarga este repositorio o clónalo y abre PowerShell en su carpeta. Ejecuta `./iniciar.ps1` y abre `http://127.0.0.1:8080`. Se necesita un compilador C++17; el script busca g++ o el MinGW de CodeBlocks. Node.js solo es necesario para las pruebas JavaScript y HTTP.

El primer arranque copia `data/mapa-inicial.txt` a `data/mapa.txt` si todavía no tienes un mapa de trabajo. Incluye cinco lugares y tres conexiones aproximadas. Tus cambios posteriores, respaldos y ejecutables permanecen en tu computadora y no se suben automáticamente al repositorio.

GitHub conserva el proyecto; esta versión se ejecuta con su servidor C++ local. Subir los archivos no publica una aplicación web en Internet.

**Extra Unity en espera:** los archivos iniciales están en [unity/README.md](unity/README.md). La animación todavía no está compilada ni conectada a la web; su prueba requiere activar una licencia en Unity Hub. La aplicación principal funciona sin Unity.

## Estado actual

La web administra el grafo real de C++. Permite:

- Registrar, buscar por nombre/categoría/ID, editar y eliminar lugares.
- Crear caminos unidireccionales o bidireccionales.
- Editar distancia, tiempo y disponibilidad de un camino.
- Eliminar caminos en todos sus sentidos.
- Eliminar un lugar junto con sus conexiones entrantes y salientes.
- Ver estados de lugares y caminos, contadores y errores de validación.
- Calcular rutas por distancia o tiempo, con lugares en orden, detalle de tramos y ambos totales.

**Guardado automático:** cada alta, edición o eliminación se guarda en `data/mapa.txt`. Al iniciar se carga y valida ese archivo. El mapa conserva lugares, caminos, estados y medidas al cerrar el servidor. El esquema visual ya presenta los lugares, caminos y recorridos calculados.

La ilustración de montañas es decorativa; no representa un mapa geográfico. El mapa incluye la Laguna de los Sueños conservada y un recorrido inicial Parque central → Museo Yuyay → Confites → Minas. Sus medidas son aproximadas; las notas de cada camino explican su procedencia y limitaciones.

## Ejecutar en Windows

Desde PowerShell, situado en esta carpeta:

```powershell
.\iniciar.ps1
```

El script recompila si faltan los ejecutables o si cambió el código C++. Abre http://127.0.0.1:8080 en el navegador. Mantén el programa ejecutándose y usa Ctrl+C para detenerlo. Cierra el servidor antes de recompilar.

Para compilar manualmente:

```powershell
.\compilar.ps1
```

Para utilizar otro puerto:

```powershell
.\iniciar.ps1 -Puerto 8081
```

No abras `frontend/index.html` directamente: la página necesita el servidor C++.
El script busca `g++` en PATH o utiliza `C:\Program Files\CodeBlocks\MinGW\bin\g++.exe`.
El servidor escucha únicamente en la computadora local.

## Cómo probar los formularios

1. Registra dos lugares con IDs distintos y categoría, por ejemplo dos puntos identificados como ficticios.
2. Selecciona sus nombres en el formulario de caminos y asigna distancia y tiempo positivos.
3. Elige uno o ambos sentidos y registra el camino.
4. Usa los botones de edición del catálogo para modificar los datos.
5. Comprueba la búsqueda por nombre, categoría o ID.
6. Elimina un lugar, confirma el diálogo y comprueba que sus caminos desaparecen.

Cada ID identifica un lugar o camino de forma estable durante su existencia. Se sugiere un ID disponible en los formularios, pero el servidor siempre comprueba duplicados.
La primera versión permite un solo camino lógico por pareja de lugares. Para cambiar extremos o sentido, elimina y vuelve a crear el camino. Desactivar lugares conserva sus conexiones registradas; el algoritmo excluye lugares inactivos y caminos cerrados.

## Organización

- `backend/estructuras/`: arreglo dinámico y lista enlazada propios.
- `backend/modelos/`: Lugar, Camino y Vertice.
- `backend/grafo/Grafo.hpp`: operaciones sobre lugares y conexiones.
- `backend/grafo/AlgoritmosRutas.hpp`: Dijkstra y reconstruccion de recorridos.
- `backend/api/ApiGrafo.hpp`: validación de formularios, JSON y operaciones HTTP.
- `backend/persistencia/PersistenciaMapa.hpp`: formato de archivo, carga validada, guardado y respaldo.
- `backend/main.cpp`: recuperacion inicial, arranque y servicio de la web.
- `frontend/`: HTML, CSS y JavaScript. El modulo diagrama.js presenta el esquema SVG y las rutas devueltas por C++; no calcula Dijkstra.
- `tests/`: pruebas de estructuras, grafo y API.
- `third_party/`: biblioteca HTTP y licencia.

Las estructuras propias deshabilitan copias y permiten movimientos para mantener un único dueño de cada bloque de memoria. Los caminos guardan IDs de destino, no punteros a posiciones del arreglo.

## Comprobaciones

Estructuras y grafo:

```powershell
.\probar.ps1
```

Integración HTTP (requiere Node.js y el ejecutable compilado):

```powershell
node tests/api.mjs
```

Esta prueba usa un servidor en el puerto 18080 y una carpeta temporal verificada dentro de build. No toca data/mapa.txt ni el servidor de la demostración. Comprueba también reinicios, archivos corruptos y fallos al guardar, y retira sus datos al terminar.

Comprobado: compilación con MinGW GCC 14.2; pruebas de estructuras y grafo; altas, ediciones y eliminaciones HTTP; JSON con acentos, comillas y saltos de línea; campos e IDs inválidos; pesos no finitos; duplicados; limpieza de conexiones; solicitudes simultáneas. Flujo de registro, edición, búsqueda, caminos en ambos sentidos y eliminación revisado también desde el navegador con datos temporales de prueba, retirados al finalizar.

El contador de objetos de las pruebas comprueba liberación de recursos en esos escenarios; no equivale a un análisis completo con un detector de errores de memoria.

## API de esta etapa

Las respuestas son JSON. Las altas y ediciones reciben `application/x-www-form-urlencoded`. Las modificaciones llevan la cabecera `X-Salinas-Client: web`. La página utiliza estas mismas operaciones.

| Método | Ruta | Operación |
|---|---|---|
| GET | `/api/ruta?origen={id}&destino={id}&criterio=distancia` | Ruta optima; criterio distancia o tiempo |
| GET | `/api/estado` | Estado del servidor |
| GET | `/api/mapa` | Lugares, caminos lógicos y cantidad de arcos en una consulta |
| GET | `/api/lugares` | Catálogo de lugares |
| GET | `/api/caminos` | Caminos sin duplicar los bidireccionales |
| POST | `/api/guardar` | Guardar el mapa actual |
| POST | `/api/cargar` | Validar el archivo y reemplazar el mapa actual |
| POST | `/api/lugares` | Registrar lugar: id, nombre, descripcion, categoria, activo |
| PUT | `/api/lugares/{id}` | Editar nombre, descripcion, categoria, activo |
| DELETE | `/api/lugares/{id}` | Eliminar lugar y conexiones |
| POST | `/api/caminos` | Registrar: id, origen, destino, distancia, tiempo, bidireccional, habilitado |
| PUT | `/api/caminos/{id}` | Editar distancia, tiempo, habilitado |
| DELETE | `/api/caminos/{id}` | Eliminar todos los arcos del camino |

Los booleanos se envían como `true` o `false`. Distancias en metros y tiempos en minutos. Un mutex protege las consultas y modificaciones concurrentes del grafo.

## Dependencias

- Compilador C++17; verificado con MinGW GCC 14.2.
- Navegador actual.
- Node.js solo para las pruebas HTTP, no para utilizar la aplicación.
- cpp-httplib v0.28.0, incluida con licencia MIT en `third_party/`. Fuente: https://github.com/yhirose/cpp-httplib/tree/v0.28.0

La biblioteca se utiliza para HTTP; las estructuras académicas y Dijkstra se implementan en C++. La interfaz no depende de imágenes, fuentes ni servicios externos para funcionar.

## Planificador de recorridos

1. Registra los lugares y caminos.
2. Abre la sección Recorridos, selecciona origen, destino y criterio.
3. Pulsa Buscar recorrido. Se muestran los lugares en orden, la distancia total y el tiempo estimado; el detalle desplegable muestra cada tramo.
4. Cambiar la selección o actualizar el catálogo retira el resultado anterior para que vuelvas a calcularlo.

Se excluyen caminos cerrados y lugares inactivos, incluidos los intermedios. Se respetan los sentidos de cada conexión. Origen y destino iguales devuelven un lugar, cero metros y cero minutos. Si no existe recorrido, se informa sin inventar un camino. En empates se devuelve una de las rutas óptimas; no se aplica un segundo criterio. Los tiempos son de desplazamiento, no duración de visitas.

Las pruebas de rutas se ejecutan también con `.\probar.ps1`. Comparan los resultados con una enumeración independiente de todos los caminos simples en doce grafos pequeños, además de verificar restricciones y reconstrucción. La prueba HTTP valida la misma funcionalidad mediante `/api/ruta`. Se comprobó también desde el navegador con datos ficticios que se retiraron al finalizar.

En el ejemplo de prueba A–B–D suma 200 metros y 20 minutos, mientras A–C–D suma 600 metros y 4 minutos. Es un ejemplo académico; no representa recorridos ni medidas reales de Salinas. Una captura de esa prueba está en `docs/planificador-rutas.jpg`.

La selección del mínimo es lineal. Como cada ID vecino también se localiza mediante búsqueda lineal, esta implementación tiene costo O(V² + V·E) y espacio auxiliar O(V). Es suficiente para el catálogo pequeño previsto; incorporar un índice eficiente de IDs permitiría reducir el costo de esas búsquedas. Se usan arreglos dinámicos propios para estados, predecesores, reconstrucción y resultado.

## Guardado y recuperación

No hace falta guardar antes de salir: las modificaciones confirmadas ya están en el archivo. Los botones **Guardar mapa ahora** y **Recargar archivo guardado** permiten demostrar las operaciones desde la web. La recarga requiere confirmación y descarta las ediciones de formularios que todavía no enviaste.

- `data/mapa.txt`: mapa actual, texto UTF-8 con versión de formato.
- `data/mapa.txt.bak`: versión válida anterior al último guardado; se crea a partir del segundo guardado.
- `data/mapa.txt.lock`: marcador de bloqueo. Su existencia es normal; el sistema operativo libera el bloqueo al cerrar el servidor.
- `data/migracion-memoria-*.json`: copia adicional de los datos que estaban en la versión anterior, antes de incorporar persistencia.

La carga construye un grafo candidato y lo valida por completo antes de reemplazar el actual. El guardado escribe un temporal, lo relee para validarlo, conserva la versión anterior y reemplaza el archivo principal. Si una modificación no se puede guardar, se restaura también el grafo anterior en memoria y la operación devuelve un error. No se debe mostrar un éxito de guardado que no ocurrió.

Si el archivo principal está corrupto al iniciar, el programa muestra el error y se detiene sin arrancar vacío ni sobrescribirlo. Para recuperar un respaldo, cierra el programa, conserva una copia del archivo dañado y copia el respaldo válido sobre el principal. No modifiques los archivos mientras registras datos desde la web.

Solo un servidor puede abrir un mismo mapa a la vez. Para una instancia independiente se puede indicar otro archivo:

```powershell
.\iniciar.ps1 -Puerto 8081 -Archivo 'data/otro-mapa.txt'
```

El formato admite hasta 1.000 lugares, 10.000 caminos lógicos y archivos de 8 MiB. Los textos se escriben con comillas y escapes compatibles con `std::quoted`; pueden contener acentos, comillas, barras y saltos de línea. Las conexiones bidireccionales se guardan una sola vez y recuperan sus dos arcos.

Pruebas adicionales verificadas: preservación de textos y precisión decimal; estados; carga incompleta o inválida; IDs duplicados; caminos con destinos inexistentes; fallos de escritura sin cambios parciales; bloqueo de doble apertura; recuperación después de terminar y reiniciar el proceso. El lugar que ya estaba registrado se migró y se comparó con la copia original. El registro ficticio del ensayo de guardado fue retirado.

Referencias de la implementación de archivos en Windows: [reemplazo con MoveFileExW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw) y [apertura y exclusión de archivos](https://learn.microsoft.com/en-us/windows/win32/fileio/creating-and-opening-files).

## Esquema visual de lugares y caminos

La sección **Esquema** muestra un dibujo SVG de los lugares y sus conexiones. Es un esquema: la posición de los puntos, la longitud de las líneas y sus cruces no representan geografía, medidas ni intersecciones reales. Los lugares se ordenan por ID y se distribuyen automáticamente en un círculo.

- Cada punto se puede seleccionar para ver el nombre completo, categoría, descripción, estado y conexiones salientes.
- **Usar como origen** y **Usar como destino** completan los campos del planificador. Los lugares inactivos se muestran, pero no se pueden elegir como extremos.
- Las flechas indican el sentido de cada conexión; un camino bidireccional tiene dos flechas.
- Los caminos cerrados o conectados a lugares inactivos aparecen discontinuos y grises.
- Al calcular un recorrido, se resaltan sus lugares y caminos en color ocre. La flecha del resaltado indica el sentido utilizado, incluso al recorrer un camino bidireccional al revés.
- Cambiar la selección, actualizar el catálogo o modificar el mapa retira el resaltado anterior.
- Los botones de zoom amplían la vista hasta el 200 %. El esquema se puede desplazar dentro de su área y restablecer al 100 %.
- Los puntos admiten selección con Tab y Enter o Espacio. Los nombres largos se abrevian en el dibujo y se muestran completos en el panel de detalle.

No se agregaron bibliotecas ni servicios externos para el dibujo. Se utilizan SVG y JavaScript; los datos, estructuras académicas, algoritmos y archivos continúan en C++. El archivo frontend/package.json solo declara el uso de módulos JavaScript, sin dependencias.

Pruebas de la representación:

```powershell
node --test tests/diagrama.mjs
```

Verifican distribución estable sin modificar datos, posiciones finitas, sentido inverso de caminos bidireccionales, cierres, lugares inactivos, referencias ausentes y limpieza de resaltados. Se comprobaron en el navegador selección de lugares y extremos, resultados distintos por criterio, rutas sin desplazamiento o inexistentes, flechas, zoom y teclado. Los ejemplos se ejecutaron con otro archivo y puerto, separados de data/mapa.txt. La captura docs/esquema-ruta.jpg utiliza esos datos ficticios de prueba.

## Datos aproximados incorporados (20 de noviembre de 2026)

| Camino | Metros | Minutos a pie |
|---|---|---|
| Parque central → Museo Yuyay | 250 | 6 |
| Museo Yuyay → Confites | 50 | 2 |
| Confites → Minas | 250 | 8 |

Estimaciones aceptadas por el usuario para el proyecto; no son mediciones de campo. Para el parque se usó la iglesia como referencia. Para las minas se escogió un valor central del rango de trabajo 200–300 m y 6–10 min; Google Maps no trazó todo el sendero. Se cargaron como conexiones bidireccionales con iguales pesos para ida y vuelta, una simplificación académica que debe revisarse localmente. El recorrido completo suma aproximadamente 550 m y 16 min, sin visitas. Con estas tres conexiones solo existe un itinerario entre los extremos, por lo que ambos criterios eligen el mismo. Las pruebas de rutas y API incluyen grafos ficticios donde los criterios sí difieren.

La Laguna de los Sueños mantiene su descripción original y queda sin conexiones hasta contar con medidas. Por ello es correcto que el planificador informe que no hay ruta hacia ella desde el circuito.

Las notas se pueden editar en Caminos y aparecen en los resultados y el detalle del esquema. Se guardan en el formato SALINAS_MAPA 2; el programa también lee archivos anteriores de versión 1. Copiar un archivo versión 2 a una versión antigua del ejecutable no está soportado. Fuentes y capturas: [mediciones](docs/datos/MEDICIONES_MAPA.md).
