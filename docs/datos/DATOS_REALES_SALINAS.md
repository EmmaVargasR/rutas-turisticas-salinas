# Datos turísticos de Salinas: investigación y validación local

Consulta: 20 de noviembre de 2026. Ámbito: Salinas de Guaranda, Bolívar, Ecuador. Modo del proyecto: a pie.

Estado: catálogo documentado y conexiones propuestas para revisar. Se consultaron tres recorridos en Google Maps; los resultados, sustituciones de referencia y limitaciones están en [MEDICIONES_MAPA.md](MEDICIONES_MAPA.md). El usuario aceptó las estimaciones y se incorporaron tres conexiones como aproximadas. La validación de campo sigue pendiente. El lugar original se conservó y se guardó una copia previa del mapa.

## Catálogo propuesto

Los IDs son provisionales para esta propuesta; el ID 1 mantiene la Laguna de los Sueños. La presencia en una fuente documenta el lugar, no confirma su disponibilidad actual para visitas.

| ID | Lugar o punto de paso | Categoría | Fuente |
|---|---|---|---|
| 1 | Laguna de los Sueños | Natural | [Turismo local](https://www.salinasdeguaranda.com/lugares-turisticos-de-salinas/) |
| 2 | Parque central de Salinas | Punto de paso | [Referencias de acceso](https://www.salinasdeguaranda.com/lugares-turisticos-de-salinas/) |
| 3 | Iglesia de Salinas | Cultural / referencia de acceso | [Referencias de acceso](https://www.salinasdeguaranda.com/lugares-turisticos-de-salinas/) |
| 4 | Museo Salinas Yuyay | Cultural | [Fundación Familia Salesiana](https://ffss.ec/museo-salinas-yuyay/), [dirección publicada](https://www.salinasdeguaranda.com/museo-salinas/) |
| 5 | Fábrica de Confites El Salinerito | Productivo | [Ficha del emprendimiento](https://www.salinasdeguaranda.com/confites-salinerito/) |
| 6 | Minas de Sal / Vertientes de Agua Sal | Histórico | [Ministerio de Turismo](https://historicomintur.produccion.gob.ec/dos-nuevos-rincones-magicos-del-ecuador-se-declararon-en-guaranda/) |
| 7 | Quesera El Salinerito | Productivo | [Cooperativa de Producción Agropecuaria](https://salinerito.com/gruppo-salinas/cooperativa-de-produccion-agropecuaria-el-salinerito/) |
| 8 | TEXAL Salinas | Artesanal | [Corporación Gruppo Salinas](https://salinerito.com/gruppo-salinas/texal-salinerito/) |
| 9 | Mirador La Cruz y Columpio de las Alturas | Natural | [Turismo local](https://www.salinasdeguaranda.com/lugares-turisticos-de-salinas/) |
| 10 | Hilandería Intercomunal Salinas | Productivo | [Ministerio de Turismo](https://historicomintur.produccion.gob.ec/dos-nuevos-rincones-magicos-del-ecuador-se-declararon-en-guaranda/), [operador local](https://www.tomabelas.com/) |

Las descripciones y fuentes por registro están en `propuesta-salinas.json`. Mirador y columpio se agrupan inicialmente como un sitio; si sus entradas son distintas, hay que separarlos y medir el tramo. La iglesia y el parque funcionan también como referencias del grafo.

## Conexiones a revisar

Esta tabla es una propuesta de levantamiento, no una afirmación de que todas las aristas existen. Es necesario confirmar cada trazado antes de agregarlo al programa. En el JSON, las medidas y sentidos de las conexiones restantes siguen null. Las tres aceptadas tienen valores aproximados y una marca de incorporación. No confundir aceptación académica con comprobación de campo.

| Tramo propuesto | Qué sabemos | Distancia | Tiempo a pie | Sentido |
|---|---|---|---|---|
| Parque – Iglesia | Propuesta local por confirmar | Pendiente | Pendiente | Pendiente |
| Iglesia – Laguna | Existe una referencia de acceso publicada | Pendiente | Pendiente | Pendiente |
| Parque – Museo | Referencia provisional: iglesia | ≈250 m | ≈6 min | Ambos, simplificación |
| Museo – Confites | Estimación cartográfica aceptada | ≈50 m | ≈2 min | Ambos, simplificación |
| Confites – Minas | Hipótesis aceptada; sendero sin medir completo | ≈250 m | ≈8 min | Ambos, simplificación |
| Museo – Minas | La fundación menciona su conexión; falta fijar el trazado | Pendiente | Pendiente | Pendiente |
| Parque – Quesera | Propuesta por confirmar | Pendiente | Pendiente | Pendiente |
| Parque – TEXAL | Propuesta por confirmar | Pendiente | Pendiente | Pendiente |
| TEXAL – Hilandería | Propuesta por confirmar | Pendiente | Pendiente | Pendiente |
| Parque – Mirador La Cruz | Existe una referencia del acceso publicada | Pendiente | Pendiente | Pendiente |
| Quesera – TEXAL | Alternativa que debe verificarse o descartarse | Pendiente | Pendiente | Pendiente |

Si una conexión pasa por otro vértice ya registrado, representarla con sus tramos correspondientes. No agregar atajos solo para conseguir que Dijkstra tenga rutas diferentes. Para peatones, una calle de un sentido vehicular no implica automáticamente un solo sentido a pie.

## Cómo completar las medidas

1. Confirmar la entrada exacta de cada lugar y la ruta caminable entre extremos.
2. Medir la longitud del recorrido sobre calles o senderos, mediante una traza GPS, una herramienta cartográfica o medición local. Anotar herramienta, fecha y trazado.
3. Registrar el tiempo caminando, sin paradas de visita. Si es aproximado, identificarlo como estimación local.
4. Si solo se dispone de distancia, puede calcularse un tiempo académico provisional: minutos = metros / 50. Esto supone 3 km/h, una hipótesis elegida para el modelo; no es una velocidad medida en Salinas. No usarla para justificar la distancia ni omitir los efectos de pendiente o terreno.
5. Confirmar el regreso. El modelo actual asigna iguales pesos a ambos sentidos de un camino bidireccional: si hay diferencias importantes por pendiente, documentarlas antes de decidir si usar una estimación común o ampliar el modelo.
6. Registrar si hay acceso restringido o coordinación de visitas. Verificar la situación actual con alguien de la localidad.

Formato para responder:

`Parque → Museo | ruta/calles: ... | ... metros (medidos o estimados) | ... minutos a pie | regreso: ... | fecha/fuente: ...`

## Datos que no se deben interpretar como pesos

- Las duraciones de las fichas turísticas no identifican inequívocamente el tramo de ida utilizado en el grafo.
- Una referencia de cercanía al comienzo de un sendero no mide el recorrido completo hasta el atractivo.
- Las coordenadas de una oficina, un mapa centrado en el pueblo o un punto genérico no equivalen a la entrada del atractivo.
- La distancia en línea recta entre coordenadas no es la longitud del sendero.
- La ficha académica consultada incluye un tiempo de desplazamiento en automóvil; no se transforma automáticamente en tiempo a pie. [Investigación de la UEB, anexo de accesibilidad de Confites](https://dspace.ueb.edu.ec/server/api/core/bitstreams/f7500c8c-1ff1-4f29-837f-68d75a27b0ee/content).

## Próximos pasos

1. Revisar el catálogo y las conexiones con el usuario.
2. Completar distancias y tiempos, indicando cuáles son medidos y cuáles estimados.
3. Cargar únicamente las conexiones revisadas, conservando el lugar ya registrado.
4. Ejecutar la revisión final del proyecto con el conjunto de datos acordado.

Las tres aproximaciones ya están incorporadas. La validación de campo sigue pendiente. La revisión técnica final se documenta en ../REVISION_FINAL.md.
