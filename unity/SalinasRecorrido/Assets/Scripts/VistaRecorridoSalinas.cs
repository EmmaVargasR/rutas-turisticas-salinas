using System;
using System.Collections.Generic;
using UnityEngine;

// Paisaje y presentación. Las posiciones son ilustrativas, no coordenadas reales.
public sealed class VistaRecorridoSalinas
{
    public Transform Marcador { get; private set; }
    public LineRenderer Linea { get; private set; }
    readonly Transform raiz;
    readonly Dictionary<int, Vector3> posiciones;
    readonly Dictionary<Color, Material> materiales = new Dictionary<Color, Material>();
    readonly List<UnityEngine.Object> recursos = new List<UnityEngine.Object>();
    readonly Color bosque = new Color(0.12f, 0.25f, 0.20f);
    readonly Color papel = new Color(0.96f, 0.96f, 0.92f);
    readonly Color oro = new Color(0.91f, 0.62f, 0.22f);
    readonly Color tintaSuave = new Color(0.35f, 0.43f, 0.36f);
    Camera camara;
    GUIStyle titulo, subtitulo, texto, pequeno, cifra, nombre, etiqueta, boton, secundario;
    float escala, ancho, alto;
    bool compacto;

    public VistaRecorridoSalinas(Transform padre, Dictionary<int, Vector3> ubicaciones)
    {
        raiz = padre; posiciones = ubicaciones;
        CrearMundo(); AjustarCamara();
    }

    Material MaterialDe(Color color)
    {
        Material resultado;
        if (materiales.TryGetValue(color, out resultado)) return resultado;
        var shader = Shader.Find("Standard");
        if (shader == null) shader = Shader.Find("Unlit/Color");
        resultado = new Material(shader) { color = QualitySettings.activeColorSpace == ColorSpace.Linear ? color.linear : color };
        if (resultado.HasProperty("_Glossiness")) resultado.SetFloat("_Glossiness", 0.08f);
        materiales.Add(color, resultado); recursos.Add(resultado); return resultado;
    }

    GameObject Figura(string nombreObjeto, PrimitiveType tipo, Vector3 p, Vector3 tamano, Color color)
    {
        var objeto = GameObject.CreatePrimitive(tipo); objeto.name = nombreObjeto;
        objeto.transform.SetParent(raiz); objeto.transform.position = p; objeto.transform.localScale = tamano;
        objeto.GetComponent<Renderer>().sharedMaterial = MaterialDe(color);
        var colision = objeto.GetComponent<Collider>(); if (colision != null) UnityEngine.Object.Destroy(colision);
        return objeto;
    }

    GameObject Malla(string nombreObjeto, Vector3 p, Vector3[] vertices, int[] triangulos, Color color)
    {
        var mesh = new Mesh { name = nombreObjeto, vertices = vertices, triangles = triangulos };
        mesh.RecalculateNormals(); mesh.RecalculateBounds(); recursos.Add(mesh);
        var objeto = new GameObject(nombreObjeto); objeto.transform.SetParent(raiz); objeto.transform.position = p;
        objeto.AddComponent<MeshFilter>().sharedMesh = mesh;
        objeto.AddComponent<MeshRenderer>().sharedMaterial = MaterialDe(color); return objeto;
    }

    void Tejado(Vector3 p, float anchoTecho, float fondo, Color color)
    {
        float x = anchoTecho / 2, z = fondo / 2;
        Malla("Tejado de dos aguas", p, new[] {
            new Vector3(-x,0,-z), new Vector3(x,0,-z), new Vector3(x,0,z),
            new Vector3(-x,0,z), new Vector3(0,0.65f,-z), new Vector3(0,0.65f,z)
        }, new[] {0,4,1, 3,2,5, 0,3,5, 0,5,4, 1,4,5, 1,5,2}, color);
    }

    void Farallon(Vector3 p, float radio, float altura, Color color)
    {
        const int lados = 7;
        var vertices = new Vector3[lados * 2 + 1];
        var triangulos = new List<int>();
        for (int i = 0; i < lados; i++) {
            float a = i * Mathf.PI * 2 / lados;
            float r = radio * (i % 2 == 0 ? 1 : 0.88f);
            vertices[i] = new Vector3(Mathf.Cos(a)*r, 0, Mathf.Sin(a)*r);
            vertices[i+lados] = new Vector3(Mathf.Cos(a)*r*0.72f, altura*(i%2==0?1:0.94f), Mathf.Sin(a)*r*0.72f);
            int j = (i+1)%lados;
            triangulos.AddRange(new[] {i,j+lados,j, i,i+lados,j+lados, i+lados,lados*2,j+lados});
        }
        vertices[lados*2] = Vector3.up*altura;
        var caras = new Vector3[triangulos.Count];
        var indices = new int[caras.Length];
        for(int i=0;i<caras.Length;i++) { caras[i]=vertices[triangulos[i]]; indices[i]=i; }
        Malla("Farallón ilustrativo", p, caras, indices, color);
        var cima = new Vector3[lados+1];
        var superficie = new List<int>();
        for(int i=0;i<lados;i++) {
            cima[i]=vertices[i+lados]+Vector3.up*.025f;
            superficie.AddRange(new[] {i,lados,(i+1)%lados});
        }
        cima[lados]=vertices[lados*2]+Vector3.up*.025f;
        Malla("Vegetación del farallón",p,cima,superficie.ToArray(),new Color(.30f,.43f,.23f));
    }

    void Arbol(Vector3 p, float altura)
    {
        Figura("Tronco", PrimitiveType.Cylinder, p+Vector3.up*altura*0.28f, new Vector3(.16f,altura*.28f,.16f), new Color(.38f,.29f,.19f));
        Figura("Copa", PrimitiveType.Sphere, p+Vector3.up*altura*.73f, new Vector3(altura*.7f,altura*.92f,altura*.7f), new Color(.20f,.38f,.25f));
    }

    void Edificio(Vector3 p, bool confites)
    {
        var crema = new Color(.88f,.80f,.64f);
        Figura(confites?"Local de Confites":"Museo ilustrativo", PrimitiveType.Cube, p+Vector3.up*.65f, new Vector3(2.2f,1.3f,1.6f), crema);
        Tejado(p+Vector3.up*1.3f,2.55f,1.95f,confites?new Color(.39f,.23f,.16f):new Color(.61f,.28f,.19f));
        Figura("Puerta", PrimitiveType.Cube, p+new Vector3(0,.45f,-.82f), new Vector3(.42f,.9f,.07f), bosque);
        foreach (float x in new[] {-.72f,.72f})
            Figura("Ventana",PrimitiveType.Cube,p+new Vector3(x,.8f,-.83f),new Vector3(.44f,.43f,.08f),new Color(.37f,.61f,.64f));
        if (confites) {
            Figura("Marquesina",PrimitiveType.Cube,p+new Vector3(0,1.12f,-1f),new Vector3(1.7f,.14f,.55f),oro);
            Figura("Chimenea",PrimitiveType.Cube,p+new Vector3(.65f,1.75f,.35f),new Vector3(.28f,.9f,.3f),crema);
        } else {
            for(int i=0;i<3;i++) Figura("Escalón del museo",PrimitiveType.Cube,p+new Vector3(0,.04f*i,-1.15f+.12f*i),new Vector3(1,.08f,.65f-.12f*i),new Color(.68f,.68f,.58f));
        }
    }

    void CrearMundo()
    {
        var fondo = new GameObject("Fondo").AddComponent<Camera>(); fondo.transform.SetParent(raiz);
        fondo.depth = -10; fondo.cullingMask = 0; fondo.clearFlags = CameraClearFlags.SolidColor; fondo.backgroundColor = papel;
        camara = new GameObject("Camara del recorrido").AddComponent<Camera>(); camara.transform.SetParent(raiz);
        camara.transform.position = new Vector3(0,14,-18); camara.transform.LookAt(new Vector3(0,.5f,.5f));
        camara.orthographic = true; camara.backgroundColor = papel; camara.clearFlags = CameraClearFlags.SolidColor;
        var luz = new GameObject("Luz suave").AddComponent<Light>(); luz.transform.SetParent(raiz);
        luz.type = LightType.Directional; luz.intensity = .80f; luz.transform.rotation = Quaternion.Euler(45,-30,0);
        luz.shadows = LightShadows.Soft;
        RenderSettings.ambientMode = UnityEngine.Rendering.AmbientMode.Flat;
        RenderSettings.ambientLight = new Color(.40f,.44f,.38f);
        Figura("Base de tierra",PrimitiveType.Cube,new Vector3(0,-.4f,0),new Vector3(20,.65f,11),new Color(.43f,.46f,.32f));
        Figura("Pradera",PrimitiveType.Cube,new Vector3(0,-.055f,0),new Vector3(20,.12f,11),new Color(.55f,.65f,.39f));
        Farallon(new Vector3(-5,0,3.1f),2.1f,3.1f,new Color(.43f,.45f,.38f));
        Farallon(new Vector3(2.8f,0,3.4f),1.9f,2.8f,new Color(.47f,.49f,.41f));
        foreach (var p in new[] {new Vector3(-8,0,2),new Vector3(-8.6f,0,-.5f),new Vector3(-7,0,3.8f),new Vector3(-1,0,4.3f),new Vector3(0,0,3.9f),new Vector3(7.4f,0,3),new Vector3(8.7f,0,1.2f),new Vector3(8.8f,0,-2.6f)}) Arbol(p,1.4f);
        var parque = posiciones[2];
        Figura("Plaza circular",PrimitiveType.Cylinder,parque+Vector3.up*.06f,new Vector3(3,.07f,3),new Color(.81f,.78f,.64f));
        Figura("Fuente",PrimitiveType.Cylinder,parque+new Vector3(-.65f,.22f,.4f),new Vector3(1.3f,.16f,1.3f),new Color(.58f,.61f,.54f));
        Figura("Agua de la fuente",PrimitiveType.Cylinder,parque+new Vector3(-.65f,.39f,.4f),new Vector3(1.08f,.025f,1.08f),new Color(.34f,.65f,.68f));
        Figura("Centro de la fuente",PrimitiveType.Sphere,parque+new Vector3(-.65f,.53f,.4f),Vector3.one*.35f,new Color(.81f,.81f,.69f));
        Arbol(parque+new Vector3(-1.5f,0,.7f),1.1f); Arbol(parque+new Vector3(1.4f,0,.9f),1.2f);
        Edificio(posiciones[4]+Vector3.forward*1.25f,false); Edificio(posiciones[5]+Vector3.forward*1.25f,true);
        var minas = posiciones[6]+Vector3.forward*1.2f;
        for(int i=0;i<4;i++) {
            Figura("Terraza salinera",PrimitiveType.Cube,minas+new Vector3(0,.08f+i*.13f,.15f*i),new Vector3(2.8f-i*.35f,.15f,2.2f-i*.3f),new Color(.82f,.79f,.66f));
        }
        Figura("Poza salinera",PrimitiveType.Cylinder,minas+new Vector3(0,.62f,.4f),new Vector3(1.2f,.025f,.9f),new Color(.79f,.62f,.30f));
        for(int i=0;i<3;i++) Figura("Sal",PrimitiveType.Sphere,minas+new Vector3(-.8f+i*.55f,.55f,-.5f),new Vector3(.65f,.45f,.5f),new Color(.94f,.93f,.84f));
        Linea = new GameObject("Ruta de C++").AddComponent<LineRenderer>(); Linea.transform.SetParent(raiz);
        var matLinea = new Material(Shader.Find("Sprites/Default")); recursos.Add(matLinea);
        Linea.sharedMaterial = matLinea; Linea.widthMultiplier = .18f; Linea.numCapVertices = 6;
        Linea.startColor = Linea.endColor = oro; Linea.positionCount = 0;
        var persona = new GameObject("Visitante"); persona.transform.SetParent(raiz); Marcador = persona.transform;
        foreach(var pieza in new[] {
            Figura("Cuerpo",PrimitiveType.Capsule,new Vector3(0,-.25f,0),new Vector3(.32f,.33f,.3f),bosque),
            Figura("Cabeza",PrimitiveType.Sphere,new Vector3(0,.28f,0),Vector3.one*.36f,new Color(.80f,.61f,.43f)),
            Figura("Mochila",PrimitiveType.Cube,new Vector3(0,-.14f,.22f),new Vector3(.30f,.36f,.2f),oro),
            Figura("Pie izquierdo",PrimitiveType.Cube,new Vector3(-.12f,-.77f,-.02f),new Vector3(.16f,.25f,.28f),bosque),
            Figura("Pie derecho",PrimitiveType.Cube,new Vector3(.12f,-.77f,-.02f),new Vector3(.16f,.25f,.28f),bosque)
        }) pieza.transform.SetParent(Marcador,false);
        // Las piezas se crearon en coordenadas locales alrededor del origen.
    }

    public void AjustarCamara()
    {
        compacto = Screen.width < Screen.height * 1.25f;
        escala = Mathf.Max(.5f,Mathf.Min(Screen.width/(compacto?700f:1280f),Screen.height/720f));
        ancho = Screen.width/escala; alto = Screen.height/escala;
        camara.rect = compacto ? new Rect(0,.32f,1,.49f) : new Rect(.01f,.19f,.73f,.62f);
        float aspecto = Screen.width*camara.rect.width/(Screen.height*camara.rect.height);
        camara.orthographicSize = Mathf.Max(5.1f,11f/aspecto);
    }

    Texture2D Fondo(Color color)
    {
        var textura = new Texture2D(1,1); textura.SetPixel(0,0,color); textura.Apply(); recursos.Add(textura); return textura;
    }

    GUIStyle Estilo(int tamano, Color color, bool negrita = false)
    {
        var estilo = new GUIStyle(GUI.skin.label) {fontSize=tamano,fontStyle=negrita?FontStyle.Bold:FontStyle.Normal,wordWrap=true};
        estilo.normal.textColor=color; return estilo;
    }

    void PrepararEstilos()
    {
        if(titulo!=null)return;
        titulo=Estilo(34,bosque,true); subtitulo=Estilo(15,tintaSuave); texto=Estilo(18,bosque);
        pequeno=Estilo(14,tintaSuave); cifra=Estilo(30,bosque,true); nombre=Estilo(17,bosque,true);
        etiqueta=Estilo(14,bosque,true); etiqueta.alignment=TextAnchor.MiddleCenter;
        boton=new GUIStyle(GUI.skin.button){fontSize=18,fontStyle=FontStyle.Bold,border=new RectOffset(),padding=new RectOffset(12,12,8,8)};
        boton.normal.background=Fondo(bosque); boton.hover.background=Fondo(new Color(.23f,.38f,.27f));
        boton.active.background=Fondo(new Color(.10f,.20f,.16f));
        boton.normal.textColor=boton.hover.textColor=boton.active.textColor=Color.white;
        secundario=new GUIStyle(boton); secundario.normal.background=Fondo(new Color(.88f,.91f,.83f));
        secundario.normal.textColor=bosque;
    }

    void Rectangulo(Rect rect, Color color)
    {
        Color anterior=GUI.color; GUI.color=color; GUI.DrawTexture(rect,Texture2D.whiteTexture); GUI.color=anterior;
    }
    void Panel(Rect rect)
    {
        Rectangulo(new Rect(rect.x+2,rect.y+3,rect.width,rect.height),new Color(.12f,.25f,.20f,.07f));
        Rectangulo(rect,new Color(.82f,.86f,.78f)); Rectangulo(new Rect(rect.x+1,rect.y+1,rect.width-2,rect.height-2),Color.white);
    }

    public void Dibujar(RecorridoSalinas.Ruta ruta,int tramoActual,float metros,float minutos,bool reproduciendo,string estado,string error,Action reproducir,Action reiniciar)
    {
        PrepararEstilos();
        GUI.matrix=Matrix4x4.Scale(new Vector3(escala,escala,1));
        GUI.Label(new Rect(28,18,ancho-56,24),"SALINAS DE GUARANDA  /  CAMINOS POR DESCUBRIR",subtitulo);
        GUI.Label(new Rect(26,47,ancho-52,48),"Tu recorrido, paso a paso.",titulo);
        GUI.Label(new Rect(28,99,ancho-56,25),"Vista previa · recorrido de ejemplo · animación acelerada",pequeno);
        if (ruta!=null) {
            foreach(var par in posiciones) {
                var punto=camara.WorldToScreenPoint(par.Value+Vector3.up*2.25f);
                var rect=new Rect(punto.x/escala-80,(Screen.height-punto.y)/escala-12,160,28);
                Rectangulo(rect,new Color(.98f,.98f,.94f,.95f));
                GUI.Label(rect,NombreCorto(par.Key),etiqueta);
            }
        }
        float x=compacto?24:ancho-330, y=compacto?alto*.70f:142;
        float w=compacto?ancho-48:304;
        if(!compacto) {
            Panel(new Rect(x,y,w,alto-y-139));
            GUI.Label(new Rect(x+20,y+18,w-40,24),"TU RECORRIDO",subtitulo);
            if(ruta!=null) {
                GUI.Label(new Rect(x+20,y+51,130,42),ruta.distanciaTotal.ToString("0")+" m",cifra);
                GUI.Label(new Rect(x+161,y+51,125,42),ruta.tiempoTotal.ToString("0")+" min",cifra);
                GUI.Label(new Rect(x+20,y+94,w-40,23),"Distancia y tiempo aproximados",pequeno);
                Rectangulo(new Rect(x+20,y+132,w-40,1),new Color(.86f,.89f,.82f));
                for(int i=0;i<ruta.lugares.Length;i++) {
                    float fila=y+151+i*48;
                    Rectangulo(new Rect(x+20,fila,28,28),i<=tramoActual?new Color(.84f,.91f,.70f):new Color(.93f,.94f,.90f));
                    GUI.Label(new Rect(x+20,fila,28,28),(i+1).ToString(),etiqueta);
                    GUI.Label(new Rect(x+60,fila+3,w-76,40),NombreCorto(ruta.lugares[i].id),nombre);
                }
            }
        }
        float barraY=alto-116;
        Panel(new Rect(24,barraY,ancho-48,90));
        GUI.enabled=ruta!=null&&ruta.tramos.Length>0;
        if(GUI.Button(new Rect(40,barraY+17,168,42),reproduciendo?"Pausar":"Reproducir",boton)) reproducir();
        if(GUI.Button(new Rect(220,barraY+17,125,42),"Reiniciar",secundario)) reiniciar();
        GUI.enabled=true;
        float inicio=compacto?370:380;
        if(ancho>650) {
            GUI.Label(new Rect(inicio,barraY+12,ancho-inicio-48,26),string.IsNullOrEmpty(error)?estado:error,texto);
            float progreso=ruta!=null&&ruta.tiempoTotal>0?Mathf.Clamp01(minutos/ruta.tiempoTotal):0;
            float largo=Mathf.Max(20,ancho-inicio-72);
            Rectangulo(new Rect(inicio,barraY+48,largo,7),new Color(.88f,.91f,.84f));
            Rectangulo(new Rect(inicio,barraY+48,largo*progreso,7),oro);
            GUI.Label(new Rect(inicio,barraY+62,largo,22),string.Format("{0:0} m recorridos   ·   {1:0.0} min aproximados   ·   {2:0}%",metros,minutos,progreso*100),pequeno);
        }
        GUI.Label(new Rect(28,alto-22,ancho-56,22),"Representación ilustrativa, sin escala geográfica. Los tiempos no incluyen visitas.",pequeno);
        GUI.matrix=Matrix4x4.identity;
    }

    static string NombreCorto(int id)
    {
        switch(id) {case 2:return "Parque central";case 4:return "Museo Yuyay";case 5:return "Confites";case 6:return "Minas de Sal";default:return "Lugar "+id;}
    }

    public void Liberar()
    {
        foreach(var recurso in recursos) if(recurso!=null) UnityEngine.Object.Destroy(recurso);
    }
}
