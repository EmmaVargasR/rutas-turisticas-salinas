# Animación del recorrido: primera etapa

Proyecto Unity: `SalinasRecorrido`. Versión detectada: Unity 6000.6.4f1.

## Estado real

Preparados el controlador, el generador de escena y una copia JSON obtenida del servidor C++: Parque → Museo → Confites → Minas, 550 m y 16 min aproximados. La ejecución de Unity se detuvo antes de compilar por falta de licencia activa (salida 198). La animación está pendiente de compilación y comprobación visual. No se ha integrado ni publicado en la web.

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

- Cuatro bloques con nombres de los lugares y un marcador naranja.
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

1. Compilar y comprobar esta escena en el editor tras activar la licencia.
2. Exportar una versión Web y probarla con el servidor local.
3. Conectar el resultado actual del planificador a `RecorridoSalinas.CargarRuta` y limpiar la animación cuando cambie el recorrido.
4. Revisar errores, tamaño de pantalla y comportamiento cuando no haya ruta o la escena no represente un lugar.

No se incorporará una vista Unity incompleta a la interfaz principal. El circuito inicial solo representa los cuatro lugares con conexiones aceptadas.
