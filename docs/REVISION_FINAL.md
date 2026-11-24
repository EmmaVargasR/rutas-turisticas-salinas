# Revisión final técnica

Fecha: 20 de noviembre de 2026. Resultado: versión local funcional para demostración académica con datos aproximados. La aceptación de las estimaciones no es una comprobación de campo.

## Verificaciones realizadas

| Área | Resultado |
|---|---|
| Compilación del servidor C++17 | Correcta con compilar.ps1 |
| Arreglo dinámico y lista enlazada | Suite estructuras aprobada; propiedad de memoria y movimientos |
| Grafo | Suite grafo aprobada; altas, edición, sentidos, duplicados y eliminación de conexiones |
| Dijkstra | Suite rutas aprobada; dos criterios, cierres, inactivos, límites y comparación independiente en 12 grafos pequeños |
| Archivos | Suite persistencia aprobada; copias, validación, respaldo, bloqueo y fallos de escritura |
| Notas nuevas | Conservadas en ambos sentidos, edición sin pérdida, copia de transacción, reinicio y resultados de rutas |
| Compatibilidad | Archivos versión 1 legibles; versión 2 guarda notas y rechaza registros incompletos |
| API HTTP | tests/api.mjs aprobado con el ejecutable actualizado; incluye reinicios y rollback |
| Esquema | 5 pruebas de tests/diagrama.mjs aprobadas |
| JavaScript | Sintaxis de app.js y diagrama.js comprobada |
| Navegador | Catálogo 5 lugares / 3 caminos, notas, edición y guardado, ambos criterios y ausencia de ruta a Laguna |
| Reinicio del mapa real | Mismo catálogo, conexiones y notas recuperados; ruta de 550 m / 16 min confirmada después del reinicio |
| Vista móvil | 390 × 844; sin desbordamiento horizontal de la página (contenido 375 px y ancho útil 375 px) |

La primera ejecución de integración se lanzó mientras aún se compilaba el ejecutable actualizado y no se utilizó como resultado final. La suite posterior, con la compilación terminada, pasó.

## Conjunto de datos

- Lugar original Laguna de los Sueños conservado con su descripción exacta.
- Cuatro lugares incorporados: parque, museo, Confites y Minas.
- Tres caminos aproximados: 250 m / 6 min, 50 m / 2 min y 250 m / 8 min.
- Circuito Parque → Museo → Confites → Minas: 550 m y 16 min en ambos criterios.
- El regreso usa iguales pesos por simplificación del modelo.
- La Laguna continúa sin conexiones; la aplicación informa correctamente que no hay recorrido.
- Respaldo anterior a la carga: data/mapa-antes-estimaciones-2026-11-20.txt.
- Evidencia y fuentes: datos/MEDICIONES_MAPA.md, datos/consultas-google-maps.json y datos/datos-incorporados.json.
- Capturas: revision-final-escritorio.jpg y revision-final-movil.jpg.

## Límites y pendientes externos

1. Confirmar el punto del parque, accesos, sendero completo de Minas y tiempos de ida/vuelta.
2. Ensayar la exposición y el arranque en la computadora de entrega.
3. Incorporar informe o diapositivas solo si el profesor los solicita.
4. No se realizó un análisis completo con detector de errores de memoria. Las pruebas verifican liberación de recursos en los escenarios incluidos.

El alcance acordado usa interfaz web local, arreglo dinámico y lista enlazada propios y Dijkstra. BFS, hash, Unity y publicación en Internet no forman parte de esta entrega.
