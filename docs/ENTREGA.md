# Entrega y exposición del proyecto

Proyecto: Salinas · Caminos por descubrir. Entrega: 27 de noviembre de 2026.

## Arranque

1. Copiar la carpeta completa del proyecto, incluida data/mapa.txt y third_party.
2. Abrir PowerShell en la carpeta y ejecutar: .\iniciar.ps1
3. Abrir http://127.0.0.1:8080 y mantener el servidor abierto. Detenerlo con Ctrl+C.
4. Para recompilar se necesita un compilador C++17; el script busca g++ o el MinGW de CodeBlocks. El navegador y el servidor local no necesitan Internet para calcular recorridos. Google Maps solo se utilizó como referencia durante la preparación de datos.

## Recorrido de demostración (5–7 minutos)

1. Explicar el propósito: organizar información turística de Salinas y consultar conexiones a pie.
2. Mostrar el catálogo y las notas de los caminos. Las distancias y tiempos son aproximados, sin visitas. El parque usa la iglesia como referencia y el sendero de las minas requiere comprobación local.
3. Elegir Parque central → Minas. Calcular por distancia y por tiempo: ambos deben sumar aproximadamente 550 m y 16 min. Solo hay un itinerario en este circuito, por eso ambos criterios coinciden.
4. Abrir el detalle y ver el recorrido resaltado en el esquema. Aclarar que el esquema no es un mapa geográfico.
5. Elegir Laguna de los Sueños desde el parque: debe informar que no existe ruta registrada. No se ha inventado una conexión.
6. Explicar cómo cerrar un camino o desactivar un lugar impide usarlo en una ruta. Si se cambia algo en la exposición, restaurarlo al finalizar.
7. Explicar la persistencia: cerrar el servidor, abrirlo y comprobar que conserva los datos.

Para demostrar criterios diferentes, las pruebas tests/rutas.cpp y tests/api.mjs usan datos ficticios separados: una alternativa tiene 200 m y 20 min; otra, 600 m y 4 min. No presentar esas cifras como datos de Salinas.

## Explicación del programa

- HTML y CSS presentan la página. JavaScript envía solicitudes y dibuja el esquema.
- C++ administra los datos y calcula las rutas con Dijkstra. La interfaz no calcula el algoritmo.
- Un arreglo dinámico propio almacena los vértices; cada vértice tiene una lista enlazada propia de conexiones.
- Los caminos guardan IDs estables de los lugares. El arreglo puede crecer sin invalidar esos IDs.
- Un camino bidireccional tiene dos arcos con el mismo ID; ambos comparten sus medidas y notas.
- Dijkstra elige el vértice pendiente de menor costo con búsqueda lineal. Con la resolución lineal de IDs vecinos, el costo es O(V² + V·E) y usa O(V) de espacio auxiliar.
- El archivo se valida antes de cargar. Los cambios se guardan mediante un archivo temporal y se conserva una copia anterior. Un fallo de guardado revierte el cambio en memoria.
- cpp-httplib es la biblioteca externa para HTTP; el arreglo, lista, grafo y algoritmo son propios.

## Comprobaciones antes de presentar

- Ejecutar .\probar.ps1.
- Compilar con .\compilar.ps1 y ejecutar node tests/api.mjs.
- Ejecutar node --test tests/diagrama.mjs.
- Probar el arranque en la computadora de entrega con antelación.
- Guardar una copia de la carpeta completa con el servidor detenido.
- Si solicitan informe, presentación o un formato institucional, añadir esos documentos según las instrucciones del profesor.

## Alcance y límites

Versión académica local, sin cuentas ni publicación en Internet. No incluye Unity, navegación GPS, un mapa geográfico integrado ni disponibilidad de visitas en tiempo real. La información pública y las aproximaciones aceptadas no sustituyen el levantamiento local. La revisión de recursos en las pruebas no equivale a un análisis completo con un detector de errores de memoria.
