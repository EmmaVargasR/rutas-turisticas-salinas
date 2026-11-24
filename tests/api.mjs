import { mkdtemp, readFile, writeFile, rm, mkdir, rmdir } from 'node:fs/promises';
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

// Servidor propio y datos ficticios: no modifica un servidor abierto por el usuario.
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const base = 'http://127.0.0.1:18080';
// Cada prueba tiene su propio archivo, separado de los datos del pueblo.
const carpeta = await mkdtemp(path.join(root, 'build', 'api-test-'));
assert.equal(path.dirname(carpeta), path.join(root, 'build'));
const archivo = path.join(carpeta, 'mapa.txt');
const ejecutable = path.resolve(root, process.argv[2] || 'build/salinas.exe');
let server;
let errorArranque;
let registro = '';
function arrancar() {
  errorArranque = undefined;
  registro = '';
  server = spawn(ejecutable, ['18080', archivo], {
    cwd: root, windowsHide: true, stdio: ['ignore', 'pipe', 'pipe']
  });
  server.on('error', error => { errorArranque = error; });
  server.stdout.on('data', data => { registro += data; });
  server.stderr.on('data', data => { registro += data; });
}
async function detener() {
  if (server && server.exitCode === null && !errorArranque) {
    const terminado = new Promise(resolve => server.once('exit', resolve));
    server.kill();
    await terminado;
  }
}
async function esperar() {
  for (let intento = 0; intento < 50; intento++) {
    if (errorArranque) throw errorArranque;
    if (server.exitCode !== null) throw new Error(registro);
    if (registro.includes('Abre http')) {
      try { await peticion('/api/estado'); return; } catch { /* Esperar listen. */ }
    }
    await pausa(100);
  }
  throw new Error('El servidor no inicio: ' + registro);
}
arrancar();
const pausa = ms => new Promise(resolve => setTimeout(resolve, ms));
async function peticion(ruta, method = 'GET', datos, status = 200, headers = {}) {
  const opciones = { method, signal: AbortSignal.timeout(5000), headers: { ...headers } };
  if (method !== 'GET') opciones.headers['X-Salinas-Client'] ??= 'web';
  if (datos) {
    opciones.headers['Content-Type'] = 'application/x-www-form-urlencoded';
    opciones.body = datos instanceof URLSearchParams ? datos : new URLSearchParams(datos);
  }
  const response = await fetch(base + ruta, opciones);
  const resultado = await response.json();
  assert.equal(response.status, status, JSON.stringify(resultado));
  if (status >= 400) assert.equal(typeof resultado.error, 'string');
  return resultado;
}
const lugar = (id, nombre = `Punto ficticio ${id}`) => ({
  id, nombre, descripcion: 'Demostración: "comillas", barra \\ y salto\nÁrbol & chocolate <prueba>',
  categoria: 'Prueba', activo: true
});
const camino = (id, origen, destino, bidireccional = true) => ({
  id, origen, destino, distancia: 120.5, tiempo: 3.25, bidireccional, habilitado: true
});
try {
  let listo = false;
  for (let intento = 0; intento < 50; intento++) {
    if (errorArranque) throw errorArranque;
    if (server.exitCode !== null) throw new Error(registro);
    if (registro.includes('Abre http')) {
      try { await peticion('/api/estado'); listo = true; break; } catch { /* Esperar listen. */ }
    }
    await pausa(100);
  }
  assert.ok(listo, 'El servidor de prueba no inició: ' + registro);
  assert.equal((await peticion('/api/mapa')).lugares.length, 0);
  assert.equal((await fetch(base + '/')).status, 200);
  await peticion('/api/lugares', 'POST', lugar(1, 'Prueba "Árbol" & <texto>'), 201);
  await peticion('/api/lugares', 'POST', lugar(2), 201);
  await peticion('/api/lugares', 'POST', lugar(3), 201);
  const primero = (await peticion('/api/lugares')).lugares[0];
  assert.equal(primero.nombre, lugar(1, 'Prueba "Árbol" & <texto>').nombre);
  assert.equal(primero.descripcion, lugar(1).descripcion);
  await peticion('/api/lugares', 'POST', lugar(1), 400);
  for (const id of ['0', '-1', '1x', '1.5', '2147483648']) {
    await peticion('/api/lugares', 'POST', lugar(id), 400);
  }
  await peticion('/api/lugares', 'POST', { ...lugar(4), nombre: '   ' }, 400);
  await peticion('/api/lugares', 'POST', { ...lugar(4), activo: 'quizas' }, 400);
  const repetidos = new URLSearchParams(lugar(4)); repetidos.append('id', '5');
  await peticion('/api/lugares', 'POST', repetidos, 400);
  await peticion('/api/lugares', 'POST', lugar(4), 403, { 'X-Salinas-Client': 'otro' });
  await peticion('/api/lugares/999', 'PUT', lugar(999), 404);
  await peticion('/api/lugares/2', 'PUT', { ...lugar(2), nombre: 'Lugar editado', activo: false });
  assert.equal((await peticion('/api/mapa')).lugares.find(l => l.id === 2).activo, false);
  await peticion('/api/caminos', 'POST', camino(10, 1, 2), 201);
  await peticion('/api/caminos', 'POST', camino(20, 2, 3, false), 201);
  let mapa = await peticion('/api/mapa');
  assert.equal(mapa.caminos.length, 2); assert.equal(mapa.totalArcos, 3);
  await peticion('/api/caminos', 'POST', camino(30, 2, 1), 400);
  await peticion('/api/caminos', 'POST', camino(30, 3, 2), 400);
  await peticion('/api/caminos', 'POST', camino(30, 1, 1), 400);
  await peticion('/api/caminos', 'POST', camino(30, 1, 999), 404);
  for (const peso of ['NaN', 'Infinity', '-1', '0', '3abc', '1,5', '1e999']) {
    await peticion('/api/caminos', 'POST', { ...camino(30, 1, 3), distancia: peso }, 400);
  }
  await peticion('/api/caminos/10', 'PUT', { distancia: 200.75, tiempo: 4.5, habilitado: false });
  const editado = (await peticion('/api/caminos')).caminos.find(c => c.id === 10);
  assert.equal(editado.distancia, 200.75); assert.equal(editado.habilitado, false);
  const nota = 'Aproximado: "mapa" & <revisar>\nPendiente local';
  await peticion('/api/caminos/10', 'PUT', { distancia: 200.75, tiempo: 4.5, habilitado: true, nota });
  assert.equal((await rutaNota()).nota, nota);
  async function rutaNota() { return (await peticion('/api/caminos')).caminos.find(c => c.id === 10); }
  await peticion('/api/caminos/10', 'PUT', { distancia: 200.75, tiempo: 4.5, habilitado: true });
  assert.equal((await rutaNota()).nota, nota, 'Una actualización anterior sin nota debe conservarla');
  await peticion('/api/caminos/10', 'PUT', { distancia: 200.75, tiempo: 4.5, habilitado: true, nota: 'x'.repeat(2001) }, 400);
  assert.equal((await rutaNota()).nota, nota, 'Rechazo y rollback conservan la nota');
  await peticion('/api/caminos/10', 'DELETE');
  assert.equal((await peticion('/api/mapa')).totalArcos, 1);
  await peticion('/api/caminos/10', 'DELETE', undefined, 404);
  await peticion('/api/caminos', 'POST', camino(10, 1, 2), 201);
  await peticion('/api/lugares/2', 'DELETE');
  mapa = await peticion('/api/mapa');
  assert.equal(mapa.lugares.length, 2); assert.equal(mapa.caminos.length, 0); assert.equal(mapa.totalArcos, 0);
  // Solicitudes paralelas no deben producir IDs duplicados ni snapshots inconsistentes.
  const resultados = await Promise.all(Array.from({ length: 5 }, async () => {
    const response = await fetch(base + '/api/lugares', {
      method: 'POST', headers: { 'X-Salinas-Client': 'web', 'Content-Type': 'application/x-www-form-urlencoded' },
      body: new URLSearchParams(lugar(100))
    });
    return response.status;
  }));
  assert.equal(resultados.filter(status => status === 201).length, 1);
  assert.equal(resultados.filter(status => status === 400).length, 4);
  assert.equal((await peticion('/api/mapa')).lugares.length, 3);
  // Rutas: la menor distancia y el menor tiempo deben elegir caminos distintos.
  for (const id of [201, 202, 203, 204, 205]) await peticion('/api/lugares', 'POST', lugar(id), 201);
  for (const [id, origen, destino, distancia, tiempo, bidireccional] of [
    [201, 201, 202, 100, 10, true], [202, 202, 204, 100, 10, false],
    [203, 201, 203, 300, 2, false], [204, 203, 204, 300, 2, false]
  ]) await peticion('/api/caminos', 'POST', { ...camino(id, origen, destino, bidireccional), distancia, tiempo }, 201);
  const ruta = (origen, destino, criterio = 'distancia', status = 200) => peticion(`/api/ruta?origen=${origen}&destino=${destino}&criterio=${criterio}`, 'GET', undefined, status);
  await peticion('/api/caminos/201', 'PUT', { distancia: 100, tiempo: 10, habilitado: true, nota: 'Aproximado de prueba' });
  const corta = await ruta(201, 204);
  assert.equal(corta.tramos[0].nota, 'Aproximado de prueba');
  assert.equal((await ruta(202, 201)).tramos[0].nota, 'Aproximado de prueba');
  assert.deepEqual(corta.lugares.map(l => l.id), [201, 202, 204]);
  assert.equal(corta.distanciaTotal, 200); assert.equal(corta.tiempoTotal, 20);
  assert.equal(corta.tramos.length, 2); assert.equal(corta.tramos[0].idCamino, 201);
  assert.equal(corta.lugares[0].nombre, lugar(201).nombre);
  const rapida = await ruta(201, 204, 'tiempo');
  assert.deepEqual(rapida.lugares.map(l => l.id), [201, 203, 204]);
  assert.equal(rapida.distanciaTotal, 600); assert.equal(rapida.tiempoTotal, 4);
  const igual = await ruta(201, 201);
  assert.equal(igual.encontrada, true); assert.equal(igual.distanciaTotal, 0); assert.equal(igual.tiempoTotal, 0);
  assert.deepEqual(igual.tramos, []); assert.equal(igual.lugares.length, 1);
  assert.equal((await ruta(204, 201)).encontrada, false);
  assert.equal((await ruta(201, 205)).encontrada, false);
  await ruta(201, 9999, 'distancia', 404);
  await ruta(201, 204, 'otro', 400);
  await peticion('/api/ruta?origen=201&destino=204', 'GET', undefined, 400);
  await peticion('/api/ruta?origen=201&origen=202&destino=204&criterio=distancia', 'GET', undefined, 400);
  await peticion('/api/lugares/202', 'PUT', { ...lugar(202), activo: false });
  assert.equal((await ruta(201, 204)).distanciaTotal, 600);
  await ruta(202, 204, 'tiempo', 400);
  await peticion('/api/caminos/203', 'PUT', { distancia: 300, tiempo: 2, habilitado: false });
  const sinRuta = await ruta(201, 204);
  assert.equal(sinRuta.encontrada, false); assert.deepEqual(sinRuta.lugares, []); assert.deepEqual(sinRuta.tramos, []);
  await peticion('/api/lugares/202', 'PUT', lugar(202));
  assert.equal((await ruta(201, 204, 'tiempo')).tiempoTotal, 20);
  await peticion('/api/lugares/202', 'DELETE');
  assert.equal((await ruta(201, 204)).encontrada, false);
  // Persistencia real: los datos y las rutas sobreviven al reinicio del proceso.
  assert.equal((await peticion('/api/estado')).guardadoAutomatico, true);
  const snapshot = await peticion('/api/mapa');
  const contenidoValido = await readFile(archivo, 'utf8');
  assert.ok(contenidoValido.startsWith('SALINAS_MAPA 2'));
  assert.ok((await readFile(archivo + '.bak', 'utf8')).startsWith('SALINAS_MAPA 2'));
  await detener(); arrancar(); await esperar();
  assert.deepEqual(await peticion('/api/mapa'), snapshot);
  assert.equal((await ruta(201, 204)).encontrada, false);
  await peticion('/api/guardar', 'POST', {});
  // Recargar un archivo valido puede sustituir el mapa; un archivo roto no.
  await writeFile(archivo, 'SALINAS_MAPA 1\nLUGARES 0\nCAMINOS 0\nFIN\n', 'utf8');
  await peticion('/api/cargar', 'POST', {});
  assert.equal((await peticion('/api/mapa')).lugares.length, 0);
  await writeFile(archivo, contenidoValido, 'utf8');
  await peticion('/api/cargar', 'POST', {});
  assert.deepEqual(await peticion('/api/mapa'), snapshot);
  const corrupto = 'SALINAS_MAPA 1\nLUGARES 2\narchivo incompleto';
  await writeFile(archivo, corrupto, 'utf8');
  const respaldoPrevio = await readFile(archivo + '.bak', 'utf8');
  await peticion('/api/cargar', 'POST', {}, 400);
  await peticion('/api/lugares', 'POST', lugar(999), 400);
  await peticion('/api/guardar', 'POST', {}, 400);
  assert.deepEqual(await peticion('/api/mapa'), snapshot);
  assert.equal(await readFile(archivo, 'utf8'), corrupto);
  assert.equal(await readFile(archivo + '.bak', 'utf8'), respaldoPrevio);
  await writeFile(archivo, contenidoValido, 'utf8');
  // Un fallo del disco revierte tambien el cambio en memoria.
  await mkdir(archivo + '.tmp');
  await peticion('/api/lugares', 'POST', lugar(999), 500);
  await peticion('/api/lugares/1', 'DELETE', undefined, 500);
  await peticion('/api/guardar', 'POST', {}, 500);
  assert.deepEqual(await peticion('/api/mapa'), snapshot);
  assert.equal(await readFile(archivo, 'utf8'), contenidoValido);
  await rmdir(archivo + '.tmp');
  // Dos servidores no pueden escribir el mismo mapa simultaneamente.
  const segundo = spawn(ejecutable, ['18081', archivo], { cwd: root, windowsHide: true, stdio: 'ignore' });
  const codigoSegundo = await new Promise((resolve, reject) => {
    segundo.on('error', reject);
    segundo.on('exit', resolve);
  });
  assert.equal(codigoSegundo, 1);
  // Un archivo roto al iniciar provoca un error, sin sobrescribirlo ni arrancar vacio.
  await detener();
  await writeFile(archivo, corrupto, 'utf8');
  arrancar();
  const codigoCorrupto = await new Promise((resolve, reject) => { server.on('error', reject); server.on('exit', resolve); });
  assert.equal(codigoCorrupto, 1);
  assert.equal(await readFile(archivo, 'utf8'), corrupto);
  await writeFile(archivo, contenidoValido, 'utf8');
  arrancar(); await esperar();
  assert.deepEqual(await peticion('/api/mapa'), snapshot);
  console.log('OK: reinicio, carga validada, respaldo, bloqueo y rollback al fallar el guardado');
  console.log('OK: API de rutas por distancia/tiempo, restricciones, errores y reconstruccion');
  console.log('OK: API HTTP, formularios, JSON, validaciones, sentidos, eliminaciones y concurrencia');
} finally {
  await detener();
  // Verificar el destino absoluto antes de borrar la carpeta creada por esta prueba.
  assert.equal(path.dirname(path.resolve(carpeta)), path.join(root, 'build'));
  await rm(carpeta, { recursive: true, force: true });
}
