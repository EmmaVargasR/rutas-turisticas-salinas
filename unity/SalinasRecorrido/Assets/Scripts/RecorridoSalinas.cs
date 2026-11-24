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
    GUIStyle titulo, texto, pequeno;

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
        marcador.position = posiciones[ruta.lugares[0].id] + Vector3.up * 1.25f;
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
            marcador.position = Vector3.Lerp(posiciones[t.origen], posiciones[t.destino], avance / t.tiempo) + Vector3.up * 1.25f;
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

    Material ColorMaterial(Color color)
    {
        var shader = Shader.Find("Standard");
        if (shader == null) shader = Shader.Find("Unlit/Color");
        var material = new Material(shader); material.color = color; return material;
    }

    GameObject Figura(string nombre, PrimitiveType tipo, Vector3 posicion, Vector3 escala, Color color)
    {
        var figura = GameObject.CreatePrimitive(tipo);
        figura.name = nombre; figura.transform.SetParent(transform);
        figura.transform.position = posicion; figura.transform.localScale = escala;
        figura.GetComponent<Renderer>().sharedMaterial = ColorMaterial(color);
        return figura;
    }

    void CrearEscenario()
    {
        var camara = new GameObject("Camara del recorrido").AddComponent<Camera>();
        camara.transform.SetParent(transform);
        camara.transform.position = new Vector3(0, 12, -13);
        camara.transform.LookAt(Vector3.zero);
        camara.orthographic = true; camara.orthographicSize = 10;
        camara.clearFlags = CameraClearFlags.SolidColor;
        camara.backgroundColor = new Color(0.93f, 0.94f, 0.88f);
        var luz = new GameObject("Luz del escenario").AddComponent<Light>();
        luz.transform.SetParent(transform); luz.type = LightType.Directional;
        luz.intensity = 1.1f; luz.transform.rotation = Quaternion.Euler(50, -30, 0);
        RenderSettings.ambientLight = new Color(0.65f, 0.69f, 0.60f);
        Figura("Terreno ilustrativo", PrimitiveType.Cube, new Vector3(0, -0.2f, 0), new Vector3(18, 0.4f, 9), new Color(0.66f, 0.75f, 0.57f));
        string[] nombres = {"Parque central", "Museo Yuyay", "Confites", "Minas de Sal"};
        int[] ids = {2, 4, 5, 6};
        for (int i = 0; i < ids.Length; ++i) {
            Vector3 p = posiciones[ids[i]];
            Figura(nombres[i], PrimitiveType.Cube, p + Vector3.up * 0.4f, new Vector3(1.7f, 0.8f, 1.5f), new Color(0.18f, 0.35f, 0.28f));
            var etiqueta = new GameObject("Nombre " + nombres[i]).AddComponent<TextMesh>();
            etiqueta.transform.SetParent(transform); etiqueta.transform.position = p + new Vector3(0, 1.8f, 0);
            etiqueta.transform.rotation = camara.transform.rotation;
            etiqueta.text = nombres[i]; etiqueta.anchor = TextAnchor.MiddleCenter;
            etiqueta.alignment = TextAlignment.Center; etiqueta.fontSize = 48; etiqueta.characterSize = 0.055f;
            etiqueta.color = new Color(0.10f, 0.20f, 0.15f);
        }
        marcador = Figura("Visitante", PrimitiveType.Sphere, posiciones[2] + Vector3.up * 1.25f, Vector3.one * 0.65f, new Color(0.95f, 0.53f, 0.12f)).transform;
        recorrido = new GameObject("Recorrido recibido de C++").AddComponent<LineRenderer>();
        recorrido.transform.SetParent(transform); recorrido.useWorldSpace = true;
        recorrido.widthMultiplier = 0.12f;
        recorrido.sharedMaterial = new Material(Shader.Find("Sprites/Default"));
        recorrido.startColor = recorrido.endColor = new Color(0.89f, 0.55f, 0.12f);
        recorrido.positionCount = 0;
    }

    void OnGUI()
    {
        // Interfaz mínima sin instalar paquetes; escalada para pantallas pequeñas.
        float escala = Mathf.Min(Screen.width / 1000f, Screen.height / 700f);
        GUI.matrix = Matrix4x4.Scale(new Vector3(escala, escala, 1));
        if (titulo == null) {
            titulo = new GUIStyle(GUI.skin.label) {fontSize = 26, fontStyle = FontStyle.Bold, wordWrap = true};
            texto = new GUIStyle(GUI.skin.label) {fontSize = 18, wordWrap = true};
            pequeno = new GUIStyle(GUI.skin.label) {fontSize = 15, wordWrap = true};
        }
        GUILayout.BeginArea(new Rect(20, 15, 960, 180), GUI.skin.box);
        GUILayout.Label("SALINAS · EL RECORRIDO EN MOVIMIENTO", titulo);
        GUILayout.Label("Representación ilustrativa · medidas aproximadas · sin duración de visitas", pequeno);
        GUILayout.Label("Primera etapa: copia de una ruta calculada por C++; todavía sin conexión en vivo.", pequeno);
        GUILayout.Label(string.IsNullOrEmpty(error) ? estado : error, texto);
        if (ruta != null) GUILayout.Label(string.Format("{0:0} / {1:0} m    ·    {2:0.0} / {3:0} min aproximados", metros, ruta.distanciaTotal, minutos, ruta.tiempoTotal), texto);
        GUILayout.EndArea();
        GUILayout.BeginArea(new Rect(20, 610, 960, 80), GUI.skin.box);
        GUILayout.BeginHorizontal();
        GUI.enabled = ruta != null && ruta.tramos.Length > 0;
        if (GUILayout.Button(reproduciendo ? "Pausar" : "Reproducir", GUILayout.Height(34))) {
            if (tramoActual == ruta.tramos.Length) Reiniciar();
            reproduciendo = !reproduciendo;
        }
        if (GUILayout.Button("Reiniciar", GUILayout.Height(34))) Reiniciar();
        GUI.enabled = true;
        GUILayout.Label("Animación acelerada: 1 min de recorrido = 1,5 s de animación", pequeno);
        GUILayout.EndHorizontal();
        GUILayout.Label("Las líneas unen puntos de una escena esquemática; no trazan calles ni senderos reales.", pequeno);
        GUILayout.EndArea();
    }
}
