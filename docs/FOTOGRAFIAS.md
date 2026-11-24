# Portada y galería de Salinas

Cambio implementado y comprobado el 24 de noviembre de 2026.

## Presentación

- Portada panorámica con fotografía aportada por el usuario y acceso directo al planificador.
- Galería con las otras cuatro imágenes, textos alternativos y ampliación en un diálogo.
- Imágenes locales; no requiere un proveedor externo de fotos.
- Carga prioritaria de la portada y diferida de la galería.
- Adaptación a escritorio y móvil; respeto de la preferencia de movimiento reducido.
- Textos visuales sin asignar identidades de personas ni accesos geográficos no confirmados.
- Archivos originales conservados: comprobación SHA-256 de las cinco copias. La imagen con texto editorial se muestra completa.

## Comprobaciones

- Cinco imágenes cargadas correctamente en el navegador.
- Apertura del panorama, la foto del paisaje salinero y el retrato. Cierre con botón y Escape; foco devuelto al control inicial.
- Vista de 390 × 844: ancho útil de 375 px y contenido de 375 px; sin desbordamiento horizontal.
- Planificador desde la vista móvil: Parque → Museo → Confites → Minas, 550 m y 16 min aproximados.
- Sintaxis de galeria.js comprobada. Las cinco pruebas existentes del diagrama aprobaron.
- API HTTP: primera ejecución detenida por un fallo transitorio al reemplazar un archivo de prueba; la repetición completa pasó, incluidos guardado, reinicio y rollback. No se modificó el backend en este cambio.
- Ningún registro turístico fue modificado para probar la galería.

Capturas: portada-fotografias.jpg, galeria-fotografias.jpg y portada-fotografias-movil.jpg.

El detalle de la procedencia de los archivos está en ../frontend/assets/salinas/README.md.
