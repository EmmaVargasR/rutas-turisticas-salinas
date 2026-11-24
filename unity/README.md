# Animación del recorrido: primera etapa

Proyecto Unity: `SalinasRecorrido`. Versión detectada: Unity 6000.6.4f1.

## Estado real

Escena inicial abierta y probada desde Unity Hub con la licencia Personal del usuario. El 24 de noviembre de 2026 se renovó su presentación: edificios diferenciados, fuente, terrazas de sal, vegetación, farallones, visitante, etiquetas legibles y panel de progreso. Usa la copia JSON de la ruta de C++: Parque → Museo → Confites → Minas, 550 m y 16 min aproximados. Sigue pendiente exportarla a Web y conectarla con el planificador en vivo.

La web C++ sigue funcionando por separado. Sus datos no se modifican al abrir este proyecto Unity. La muestra JSON es una copia, no una consulta en vivo.

## Paso 1: activar Unity

1. Abrir Unity Hub e iniciar sesión.
2. Revisar Settings (engranaje) → Licenses. En algunas versiones, Licenses aparece directamente en la barra lateral.
3. Si no hay una licencia activa, usar Add license y la opción de Unity Personal que corresponda. Leer y aceptar personalmente los términos si se desea continuar.
4. Referencia: https://docs.unity.com/en-us/hub/manage-license

## Paso 2: preparar y probar la escena

Con el editor cerrado, desde la carpeta principal del proyecto:

```powershell
.\preparar-unity.ps1
```

El script debe confirmar que creó la escena y validó la muestra. Guarda los detalles en `build/unity-preparacion.log`.

En Unity Hub, ir a Projects → Add/Open → Add project from disk y seleccionar la carpeta `unity/SalinasRecorrido`. Abrir con 6000.6.4f1. Esperar la importación.

Abrir `Assets/Scenes/RecorridoSalinas.unity`. Si aún no se preparó la escena, utilizar el menú **Salinas → Preparar escena inicial**. No hace falta crear otro proyecto ni copiar los scripts manualmente.

Pulsar Play (triángulo superior) y seleccionar la pestaña Game. El escenario se construye al entrar en Play; es normal que la escena inicial solo contenga el objeto controlador antes de reproducir.

Comprobar:

- Parque con fuente, museo con tejado, local de Confites, terrazas de sal y un visitante con mochila.
- Reproducir mueve el marcador en el orden devuelto por C++.
- Pausar detiene marcador y contadores; Reproducir continúa.
- Reiniciar vuelve al parque y deja ambos contadores en cero.
- La llegada muestra 550 m y 16 min aproximados.
- Se usan 1,5 segundos de animación por cada minuto del recorrido: el circuito dura unos 24 segundos de animación. La velocidad en pantalla es ilustrativa, no una simulación física.

Las distancias de las líneas y posiciones son esquemáticas. El movimiento utiliza el tiempo de cada tramo para avanzar sus contadores; Unity no calcula Dijkstra.

## Archivos principales

- `SalinasRecorrido/Assets/Scripts/RecorridoSalinas.cs`: escenario, lectura del JSON, movimiento y controles.
- `SalinasRecorrido/Assets/Resources/ruta-ejemplo.json`: respuesta copiada de `/api/ruta?origen=2&destino=6&criterio=distancia`.
- `SalinasRecorrido/Assets/Editor/PrepararRecorrido.cs`: menú de creación de escena y validación de la muestra.

## Etapas siguientes

1. Escena compilada y comprobada en el editor; ver evidencia en ../docs/UNITY_ACABADO.md.
2. Exportar una versión Web y probarla con el servidor local.
3. Conectar el resultado actual del planificador a `RecorridoSalinas.CargarRuta` y limpiar la animación cuando cambie el recorrido.
4. Revisar errores, tamaño de pantalla y comportamiento cuando no haya ruta o la escena no represente un lugar.

No se incorporará una vista Unity incompleta a la interfaz principal. El circuito inicial solo representa los cuatro lugares con conexiones aceptadas.

## Corrección del arranque (24 de noviembre)

El script de preparación espera únicamente la salida del editor y no la de sus servicios auxiliares. Se comprobó la sintaxis de PowerShell y que el fallo de licencia termina con un mensaje claro, sin quedarse esperando. La prueba por lotes no reconocía la licencia, pero la apertura manual desde Unity Hub sí permitió compilar y ejecutar el proyecto; no interpretar aquel resultado como ausencia de una licencia Personal.

## Presentación renovada

La lógica de lectura del JSON y de movimiento sigue en RecorridoSalinas.cs. La geometría, materiales, cámara y controles se presentan en VistaRecorridoSalinas.cs. No se han incorporado assets de pago ni dependencias adicionales. Los edificios y el paisaje son ilustrativos y no reproducen fielmente entradas o coordenadas de los atractivos.

En la pestaña Game, hacer doble clic en el título Game permite ampliar la vista y repetirlo restaura la distribución del editor. Los controles mantienen la reproducción acelerada original: unos 24 segundos para el circuito de 16 minutos aproximados.
