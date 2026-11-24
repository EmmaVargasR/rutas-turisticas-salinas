using System;
using System.Collections.Generic;
using UnityEngine;

// Unity representa una ruta recibida; el cálculo de Dijkstra sigue en C++.
public class RecorridoSalinas : MonoBehaviour
{
    [Serializable] public class LugarRuta { public int id; public string nombre; }
    [Serializable] public class Tramo
    {
        public int idCamino, origen, destino;
        public float distancia, tiempo;
        public string nota;
    }
    [Serializable] public class Ruta
    {
        public bool encontrada;
        public string criterio;
        public float distanciaTotal, tiempoTotal;
        public LugarRuta[] lugares;
        public Tramo[] tramos;
    }

    readonly Dictionary<int, Vector3> posiciones = new Dictionary<int, Vector3> {
        {2, new Vector3(-6, 0, -2)}, {4, new Vector3(-2, 0, 1)},
        {5, new Vector3(2, 0, 1)}, {6, new Vector3(6, 0, -2)}
    };
    Transform marcador;
    LineRenderer recorrido;
    Ruta ruta;
    int tramoActual;
    float avance, metros, minutos;
    bool reproduciendo;
    float segundosPorMinuto = 1.5f;
    string estado = "Pulsa Reproducir para iniciar.";
    string error = "";
    VistaRecorridoSalinas vista;

    void Start()
    {
        CrearEscenario();
        var ejemplo = Resources.Load<TextAsset>("ruta-ejemplo");
        if (ejemplo == null) { error = "Falta el archivo de recorrido de ejemplo."; return; }
        CargarRuta(ejemplo.text);
    }

    public static Ruta LeerRuta(string json)
    {
        var r = JsonUtility.FromJson<Ruta>(json);
        if (r == null || !r.encontrada || r.lugares == null || r.tramos == null ||
            r.lugares.Length != r.tramos.Length + 1)
            throw new ArgumentException("No hay una ruta válida para animar.");
        float distancia = 0, tiempo = 0;
        for (int i = 0; i < r.tramos.Length; ++i) {
            var t = r.tramos[i];
            if (t == null || r.lugares[i] == null || r.lugares[i + 1] == null ||
                t.origen != r.lugares[i].id || t.destino != r.lugares[i + 1].id ||
                !FinitoPositivo(t.distancia) || !FinitoPositivo(t.tiempo))
                throw new ArgumentException("El recorrido tiene un tramo inválido.");
            distancia += t.distancia; tiempo += t.tiempo;
        }
        if (float.IsNaN(r.distanciaTotal) || float.IsInfinity(r.distanciaTotal) ||
            float.IsNaN(r.tiempoTotal) || float.IsInfinity(r.tiempoTotal) ||
            Mathf.Abs(distancia - r.distanciaTotal) > 0.1f || Mathf.Abs(tiempo - r.tiempoTotal) > 0.01f)
            throw new ArgumentException("Los totales no coinciden con los tramos.");
        return r;
    }

    static bool FinitoPositivo(float valor) { return valor > 0 && !float.IsInfinity(valor) && !float.IsNaN(valor); }

    // Este método servirá también para recibir el JSON desde la página web.
    public void CargarRuta(string json)
    {
        reproduciendo = false;
        try {
            var candidata = LeerRuta(json);
            foreach (var l in candidata.lugares)
                if (l == null || !posiciones.ContainsKey(l.id))
                    throw new ArgumentException("La escena aún no representa uno de los lugares de esta ruta.");
            ruta = candidata;
            error = "";
            recorrido.positionCount = ruta.lugares.Length;
            for (int i = 0; i < ruta.lugares.Length; ++i)
                recorrido.SetPosition(i, posiciones[ruta.lugares[i].id] + Vector3.up * 0.12f);
            Reiniciar();
        } catch (Exception e) {
            ruta = null; error = e.Message;
            if (recorrido != null) recorrido.positionCount = 0;
            if (marcador != null) marcador.gameObject.SetActive(false);
        }
    }

    void Reiniciar()
    {
        if (ruta == null) return;
        reproduciendo = false; tramoActual = 0; avance = metros = minutos = 0;
        marcador.gameObject.SetActive(true);
        marcador.position = posiciones[ruta.lugares[0].id] + Vector3.up * 0.95f;
        estado = ruta.tramos.Length == 0 ? "Ya estás en el destino." : "Pulsa Reproducir para iniciar.";
    }

    void Update()
    {
        if (!reproduciendo || ruta == null) return;
        // Consume el tiempo restante, incluso si un fotograma cruza varios tramos.
        float pendiente = Time.deltaTime / segundosPorMinuto;
        while (pendiente > 0 && tramoActual < ruta.tramos.Length) {
            var t = ruta.tramos[tramoActual];
            float consumido = Mathf.Min(pendiente, t.tiempo - avance);
            avance += consumido; pendiente -= consumido;
            minutos += consumido; metros += consumido / t.tiempo * t.distancia;
            marcador.position = Vector3.Lerp(posiciones[t.origen], posiciones[t.destino], avance / t.tiempo) + Vector3.up * 0.95f;
            estado = Nombre(t.origen) + " → " + Nombre(t.destino);
            if (avance >= t.tiempo) { ++tramoActual; avance = 0; }
        }
        if (tramoActual == ruta.tramos.Length) {
            reproduciendo = false; metros = ruta.distanciaTotal; minutos = ruta.tiempoTotal;
            estado = "Llegaste a " + Nombre(ruta.lugares[ruta.lugares.Length - 1].id) + ".";
        }
    }

    string Nombre(int id)
    {
        foreach (var l in ruta.lugares) if (l.id == id) return l.nombre;
        return "Lugar " + id;
    }

    void CrearEscenario()
    {
        vista = new VistaRecorridoSalinas(transform, posiciones);
        marcador = vista.Marcador;
        recorrido = vista.Linea;
    }

    void LateUpdate() { if (vista != null) vista.AjustarCamara(); }

    void OnGUI()
    {
        if (vista == null) return;
        vista.Dibujar(ruta, tramoActual, metros, minutos, reproduciendo, estado, error,
            () => {
                if (ruta == null) return;
                if (tramoActual == ruta.tramos.Length) Reiniciar();
                reproduciendo = !reproduciendo;
            }, Reiniciar);
    }

    void OnDestroy() { if (vista != null) vista.Liberar(); }
}
