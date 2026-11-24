# Consulta cartográfica de tres recorridos

Fecha: 20 de noviembre de 2026. Google Maps, modo Caminar. Ninguna consulta confirma por sí sola el acceso actual. Actualización posterior: las estimaciones fueron aceptadas por el usuario e incorporadas a data/mapa.txt con notas de aproximación.

| Recorrido solicitado | Resultado observado | Propuesta para revisión | Limitación |
|---|---|---|---|
| Parque central → Museo Yuyay | Iglesia → Museo: 240 m, 4 min; regreso 5 min | 250 m, 6 min | Se sustituyó el punto del parque por la iglesia como referencia provisional; confirmar el extremo real. |
| Museo Yuyay → Confites | 52 m, 1 min | 50 m, 2 min | Confirmar puertas y paso peatonal. |
| Confites → Minas | Google muestra 51 m, 1 min; descartados como medida completa | Hipótesis: 200–300 m, 6–10 min | El sendero completo no quedó medido. El rango no es una medida de Google ni un intervalo de error comprobado. |

Los valores propuestos incorporan redondeos y margen para caminar con calma; no incluyen visitas. Para las minas, los marcadores están aproximadamente a 175 m en línea recta (cálculo esférico a partir de las coordenadas visibles de los lugares). Esa separación no mide el sendero. El rango 200–300 m es solamente una hipótesis para contrastar localmente y puede quedarse corto si hay desvíos o escaleras. Se incorporó como aproximación provisional de 250 m y 8 min, aceptada por el usuario; no como distancia validada.

## Evidencia y enlaces

- [Iglesia → Museo en Google Maps](https://www.google.com/maps/dir/Iglesia+De+Salinas,+HXWJ%2B6FC,+Salinas/Salinas+Yuyay,+020107+Salinas/@-1.4054834,-79.0216782,17z/data=!3m1!4b1!4m14!4m13!1m5!1m1!1s0x91d369e270e2099d:0x753fee64bb46bd90!2m2!1d-79.0189027!2d-1.4045051!1m5!1m1!1s0x91d369deb4f9616b:0x2c272a051bc2ac82!2m2!1d-79.0192235!2d-1.4065336!3e2?authuser=0&hl=es-419&entry=ttu&g_ep=EgoyMDI2MTExNy4wIKXMDSoASAFQAw%3D%3D). Captura: maps-centro-museo.png.
- [Museo → Confites en Google Maps](https://www.google.com/maps/dir/Salinas+Yuyay,+020107+Salinas/Confites+El+Salinerito,+Salinas/@-1.4063181,-79.0219156,17z/data=!3m1!4b1!4m14!4m13!1m5!1m1!1s0x91d369deb4f9616b:0x2c272a051bc2ac82!2m2!1d-79.0192235!2d-1.4065336!1m5!1m1!1s0x91d342082138ecb1:0x915c9422324b6ece!2m2!1d-79.019559!2d-1.406133!3e2?entry=ttu&g_ep=EgoyMDI2MTExNy4wIKXMDSoASAFQAw%3D%3D). Captura: maps-museo-confites.png.
- [Confites → Minas en Google Maps](https://www.google.com/maps/dir/Confites+El+Salinerito,+Salinas/Minas+de+Sal+de+Salinas,+0205+y+Samilagua,+020107+Salinas/@-1.4068523,-79.0222078,17z/am=t/data=!4m14!4m13!1m5!1m1!1s0x91d342082138ecb1:0x915c9422324b6ece!2m2!1d-79.019559!2d-1.406133!1m5!1m1!1s0x91d36983c23faf4f:0x69488c0bd4a382f4!2m2!1d-79.0201337!2d-1.407602!3e2?entry=ttu&g_ep=EgoyMDI2MTExNy4wIKXMDSoASAFQAw%3D%3D). Captura: maps-confites-minas.png. El mapa muestra una prolongación discontinua hasta el marcador, fuera del pequeño tramo con indicaciones.

Se rechazaron coincidencias de otras localidades llamadas Salinas y del parque de Guaranda. La referencia elegida está en Salinas de Guaranda, alrededor de latitud -1.405 y longitud -79.019.

## Revisión local pendiente

1. Fijar desde qué esquina o entrada del parque se contará la ruta al museo; la consulta usa la iglesia.
2. Comprobar que el paso Museo → Confites coincide con el acceso utilizado por visitantes.
3. Medir el sendero completo desde Confites hasta el punto de las minas que se desea representar; distinguir la entrada del atractivo de las vertientes. Cronometrar ida y regreso.
4. Revisar los valores antes de incorporarlos al mapa activo.

El archivo propuesta-salinas.json distingue las tres conexiones aceptadas y cargadas de las otras pendientes. La aceptación para el proyecto no equivale a una validación de campo. Se eligieron 250 m / 6 min, 50 m / 2 min y 250 m / 8 min, con iguales pesos en ambos sentidos como simplificación académica. Respaldo previo: data/mapa-antes-estimaciones-2026-11-20.txt.
