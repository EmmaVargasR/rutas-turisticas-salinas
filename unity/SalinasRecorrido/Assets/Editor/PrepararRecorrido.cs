using System.IO;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;

public static class PrepararRecorrido
{
    [MenuItem("Salinas/Preparar escena inicial")]
    public static void CrearEscena()
    {
        const string ruta = "Assets/Scenes/RecorridoSalinas.unity";
        Directory.CreateDirectory("Assets/Scenes");
        if (!File.Exists(ruta)) {
            var escena = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            new GameObject("RecorridoSalinas").AddComponent<RecorridoSalinas>();
            EditorSceneManager.SaveScene(escena, ruta);
        } else EditorSceneManager.OpenScene(ruta);
        EditorBuildSettings.scenes = new[] {new EditorBuildSettingsScene(ruta, true)};
        var archivo = Resources.Load<TextAsset>("ruta-ejemplo");
        if (archivo == null) throw new System.Exception("Falta la ruta de ejemplo.");
        var r = RecorridoSalinas.LeerRuta(archivo.text);
        if (r.tramos.Length != 3 || r.distanciaTotal != 550 || r.tiempoTotal != 16)
            throw new System.Exception("La muestra inicial no coincide con el circuito aceptado.");
        Debug.Log("SALINAS_OK: escena preparada; JSON de C++ validado: 3 tramos, 550 m, 16 min.");
    }
}
