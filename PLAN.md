# Plan de desarrollo — Rutas turísticas de Salinas

Entrega: 27 de noviembre de 2026. Disponibilidad: 2–3 horas al día.
Meta: terminar funciones el 24, probar el 25 y preparar entrega el 26.

## Alcance acordado

Web local en HTML, CSS y JavaScript, servida por un programa C++17.
C++ administra lugares, caminos, grafo, Dijkstra y archivos. JavaScript presenta los datos; no reemplaza los algoritmos.
Estructuras propias: arreglo dinámico y lista de adyacencia enlazada. Búsqueda de IDs lineal. Sin tabla hash, heap, BFS ni cuentas en esta primera versión.
Biblioteca externa: cpp-httplib v0.28.0 (MIT), solamente para HTTP y archivos estáticos. Sus estructuras internas no forman parte de la implementación académica propia.

## Calendario

| Fecha | Entregable verificable |
|---|---|
| 14 nov | Estructura de carpetas, página inicial y comunicación web–C++ funcionando. |
| 15 nov | Arreglo dinámico propio: insertar, acceder, crecer y eliminar; pruebas. |
| 16 nov | Lista enlazada propia de conexiones; pruebas de inserción y eliminación. |
| 17 nov | Lugar, Camino, Vertice y Grafo; registrar, listar y buscar lugares. |
| 18 nov | Editar y eliminar lugares; limpiar conexiones entrantes y salientes. |
| 19 nov | Crear, editar y eliminar caminos; uno o ambos sentidos. |
| 20 nov | API y formulario web de administración integrados con el grafo. |
| 21 nov | Dijkstra por distancia y reconstrucción de ruta; comprobación manual. |
| 22 nov | Dijkstra por tiempo y planificador web; totales de ambas magnitudes. |
| 23 nov | Archivos: guardar, cargar y validar antes de reemplazar datos. |
| 24 nov | Datos locales verificados, diagrama de conexiones y diseño responsive. |
| 25 nov | Pruebas integrales, entradas incorrectas y revisión de memoria. |
| 26 nov | Correcciones, README, ensayo y copia del entregable. |
| 27 nov | Comprobación de arranque y entrega. |

## Reglas del modelo

- Un único modo de desplazamiento inicial: a pie.
- IDs estables, únicos y positivos; las conexiones guardan IDs, no punteros a elementos del arreglo.
- Metros y minutos positivos y finitos. El tiempo representa desplazamiento, no duración de visitas.
- Un lugar inactivo no participa en rutas.
- Un camino bidireccional se representa con dos arcos que comparten ID de camino; editar o eliminar afecta ambos.
- Para simplificar: no se admiten caminos paralelos ni conexiones de un punto consigo mismo.
- Origen igual a destino: recorrido de un lugar, distancia y tiempo cero.
- Los puntos de paso pueden ser vértices, además de los atractivos.
- Los datos ficticios deberán identificarse como demostración. No inventar información turística real.
- El grafo posee sus vértices; cada vértice posee sus conexiones. Evitar copias accidentales de propietarios de memoria; definir o deshabilitar operaciones de copia según el diseño.

## Organización prevista

- `backend/estructuras/`: arreglo dinámico y lista enlazada propios.
- `backend/modelos/`: Lugar, Camino, Vertice y ResultadoRuta.
- `backend/grafo/`: administración y algoritmos.
- `backend/persistencia/`: carga y guardado.
- `backend/main.cpp`: rutas HTTP y arranque.
- `frontend/`: interfaz, estilos y llamadas a la API.
- `data/`: datos persistentes, al incorporar archivos.
- `third_party/`: biblioteca HTTP y licencia.

## Criterios de entrega

1. Registrar, editar, buscar y eliminar un lugar desde la web.
2. Administrar caminos y comprobar sus sentidos.
3. Mostrar una ruta mínima por distancia y otra por tiempo con un ejemplo donde difieran.
4. Informar cuando no exista ruta o falten datos.
5. Eliminar un lugar sin dejar conexiones inválidas.
6. Reiniciar y recuperar el mismo mapa desde archivos.
7. Ejecutar desde la carpeta de entrega siguiendo el README.

## Avance

- [x] Archivos iniciales y calendario.
- [x] Verificar compilación y comunicación local (GCC 14.2, HTTP 200, API y navegador).
- [x] Arreglo dinamico propio y pruebas de crecimiento, indices y eliminacion.
- [x] Lista enlazada propia y pruebas de busqueda, eliminacion y movimiento.
- [x] Prueba conjunta: arreglo de listas y liberacion de recursos.
- [x] Modelos Lugar, Camino y Vertice; Grafo con administracion de lugares y caminos.
- [x] Pruebas del grafo: sentidos, validaciones, eliminaciones y crecimiento.
- [x] Integrar grafo con la API y los formularios web.
- [x] Administracion web: altas, ediciones, busqueda y eliminaciones con confirmacion.
- [x] Dijkstra por distancia/tiempo, reconstruccion, API y planificador web; pruebas unitarias, HTTP y navegador.
- [x] Persistencia: guardado automatico, carga al iniciar, controles web, respaldo y validacion; pruebas y reinicio real verificados.
- [x] Esquema SVG interactivo: lugares, caminos, flechas, estados, seleccion y resaltado de rutas.
- [x] Incorporar tres conexiones aproximadas aceptadas, con notas visibles y respaldo del mapa original.
- [ ] Comprobar en campo las medidas, accesos y diferencias de ida/vuelta.
- [x] Documentacion de entrega y guía de exposición en docs/ENTREGA.md.
- [ ] Ensayo del alumno y comprobación en la computadora de entrega.


## Extras posteriores

Unity queda como extra despues de completar y verificar la version principal.



## Verificacion de la administracion web

Compilacion, estructuras y grafo, API HTTP y flujo de formularios en navegador comprobados. Dijkstra, el planificador y la persistencia ya estan integrados. Los datos se conservan en archivos. Esquema visual completado. Siguiente bloque: datos locales y preparacion de entrega.


## Verificacion de rutas (15 de noviembre)

Rutas distintas por distancia y tiempo, sentidos, lugares inactivos, caminos cerrados, ausencia de ruta, origen igual al destino, errores de entrada y totales fuera del limite numerico comprobados. Comparacion independiente con enumeracion exhaustiva en doce grafos pequenos. Formularios y resultados comprobados en navegador. Datos ficticios retirados al finalizar.


## Persistencia completada

Guardado automatico por modificacion, carga inicial, guardado y recarga manual con confirmacion. Respaldo de version anterior, validacion completa antes de cargar y restauracion en memoria si falla el guardado. Bloqueo de doble apertura del mismo archivo. Pruebas de archivos, API, reinicios y navegador pasaron. El lugar existente fue conservado y se retiro el registro ficticio de ensayo.


## Esquema visual completado

Modulo SVG sin dependencias externas. Seleccion de lugares y extremos, detalle, flechas direccionales, resaltado de la ruta devuelta por C++, estados inactivos/cerrados y zoom. Pruebas de representacion y navegador superadas. Datos ficticios aislados de los lugares reales. No se modifico el archivo del usuario para probar el dibujo.


## Investigacion de datos locales (20 de noviembre)

Preparados docs/datos/DATOS_REALES_SALINAS.md y propuesta-salinas.json con 10 lugares documentados y 11 conexiones para revisar. Existencia documentada no equivale a trazados ni medidas verificados. Distancias y tiempos pendientes: null. El usuario confirmo que puede revisar recorridos y estimaciones. No se modifico data/mapa.txt. La validacion local y la revision final siguen pendientes.


## Revisión final técnica (20 de noviembre de 2026)

Compilación, cuatro suites C++, API HTTP, cinco pruebas del diagrama y revisión de navegador completadas. Se agregaron notas persistentes a los caminos con lectura compatible de archivos versión 1. Datos iniciales: cinco lugares y tres caminos aproximados, aceptados para el proyecto. El circuito suma 550 m y 16 min; Laguna conserva su contenido original y permanece sin conexión. Guía: docs/ENTREGA.md. Resultados y límites: docs/REVISION_FINAL.md. La verificación de campo y el ensayo del alumno permanecen pendientes.

## Extra Unity: animación del recorrido

El usuario eligió añadir una animación del resultado calculado por C++. Primera etapa preparada en unity/SalinasRecorrido con copia JSON real del circuito. No conectada aún a la web. La prueba del editor quedó pendiente porque Unity solicita activar la licencia desde Hub. Pasos en unity/README.md y script preparar-unity.ps1.

## Fotografías de Salinas (24 de noviembre de 2026)

Portada panorámica y galería con las cinco imágenes compartidas por el usuario. Ampliación con diálogo, navegación de teclado, carga diferida y revisión de escritorio/móvil completadas. El planificador conserva sus datos y resultados. Evidencia en docs/FOTOGRAFIAS.md. Cambio guardado como un nuevo hito de desarrollo. Unity sigue en espera.

## Acabado de Unity (24 de noviembre de 2026)

La apertura desde Hub permitió comprobar la escena con la licencia Personal del usuario. Se mejoraron paisaje, edificios, visitante, cámara, etiquetas, panel y progreso. Reproducción, pausa, continuación, llegada y reinicio comprobados en el editor. Referencias de escena verificadas. Falta configurar la exportación e integrar la animación con la web. Evidencia: docs/UNITY_ACABADO.md.
