# Acabado de la animación Unity

Comprobado el 24 de noviembre de 2026 en Unity 6000.6.4f1 abierto desde Hub por el usuario. La licencia Personal sí permite abrir y ejecutar el proyecto; el resultado anterior del arranque por lotes no representaba su estado en Hub.

## Cambio visual

Se sustituyeron cuatro bloques por un paisaje ilustrativo con parque y fuente, museo con tejado, local de Confites, terrazas de sal, farallones y árboles. Un visitante con mochila recorre las conexiones. La cámara utiliza mejor el área disponible y los nombres son etiquetas de pantalla. El panel lateral muestra las paradas y totales; los controles inferiores incluyen barra de progreso y contadores. La paleta emplea verdes, crema y ocre relacionados con la web.

La presentación está en VistaRecorridoSalinas.cs. RecorridoSalinas.cs conserva la lectura del JSON y la lógica de movimiento; cambia la altura del visitante para adaptarse al modelo. Los recursos gráficos generados se liberan al destruir el controlador. No se instalaron paquetes nuevos ni se compraron assets.

## Verificación en el editor

- Compilación y entrada en Play correctas.
- Reproducir mueve al visitante y actualiza contadores y progreso.
- Pausar lo dejó en 150 m y 3,6 min; otra observación confirmó los mismos valores y posición.
- Continuar llegó a Minas, con 550 m, 16 min y 100 %.
- Reiniciar lo devolvió al parque, con cero metros, minutos y progreso.
- Vista Game ampliada y acoplada revisadas.
- GUID del controlador referenciado por la escena y archivo meta de la escena comprobados.
- La muestra JSON conserva tres tramos, 550 m y 16 min.

Capturas: unity-recorrido-renovado.jpg y unity-llegada.jpg.

## Alcance

Escena esquemática, sin escala geográfica ni reconstrucción fiel de edificios. Cifras aproximadas, sin visitas. Usa una copia de la ruta de C++; no calcula rutas. La exportación Web, su lista de escenas, la conexión en vivo al planificador y la revisión móvil del componente están pendientes.
