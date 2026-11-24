import { DiagramaMapa } from './diagrama.js';
const $ = selector => document.querySelector(selector);
let mapa = { lugares: [], caminos: [] };
let ocupado = false;
let eliminacion = null;
const rutaForm = document.querySelector('#ruta-form');
let revisionRuta = 0;
const lugarForm = $('#lugar-form');
const caminoForm = $('#camino-form');
const campo = (form, nombre) => form.elements.namedItem(nombre);
const diagrama = new DiagramaMapa(document.querySelector('#mapa-visual'), (lugar, extremo) => {
  if (ocupado || !lugar.activo) return;
  campo(rutaForm, extremo).value = String(lugar.id);
  limpiarRuta('La selección cambió. Pulsa Buscar recorrido para actualizar el resultado.');
  mensaje(`${extremo === 'origen' ? 'Origen' : 'Destino'} seleccionado: ${lugar.nombre}.`);
});

async function solicitar(url, method = 'GET', datos) {
  const opciones = { method, signal: AbortSignal.timeout(8000), headers: {} };
  if (method !== 'GET') opciones.headers['X-Salinas-Client'] = 'web';
  if (datos) {
    opciones.headers['Content-Type'] = 'application/x-www-form-urlencoded;charset=UTF-8';
    opciones.body = new URLSearchParams(datos);
  }
  let respuesta;
  try { respuesta = await fetch(url, opciones); }
  catch { throw new Error('No se pudo confirmar la operación. Comprueba la conexión y actualiza el catálogo antes de reintentar.'); }
  const resultado = await respuesta.json();
  if (!respuesta.ok) throw new Error(resultado.error || 'No se pudo completar la operación.');
  return resultado;
}
function mensaje(texto, error = false) {
  $('#mensaje').textContent = texto;
  $('#mensaje').className = error ? 'notice error' : 'notice success';
}
function nodo(etiqueta, texto, clase) {
  const elemento = document.createElement(etiqueta);
  if (texto !== undefined) elemento.textContent = texto;
  if (clase) elemento.className = clase;
  return elemento;
}
function accion(texto, callback, peligro = false) {
  const boton = nodo('button', texto, peligro ? 'text-button danger' : 'text-button');
  boton.type = 'button';
  boton.addEventListener('click', callback);
  return boton;
}
function opciones(select, lugares, actual = select.value) {
  select.replaceChildren(new Option('Selecciona un lugar', ''));
  for (const l of lugares) select.add(new Option(`${l.nombre} · #${l.id}${l.activo ? '' : ' (inactivo)'}`, l.id));
  if (lugares.some(l => String(l.id) === actual)) select.value = actual;
}
function siguienteId(elementos) {
  return elementos.reduce((maximo, item) => Math.max(maximo, item.id), 0) + 1;
}
function nuevoLugar() {
  lugarForm.reset();
  lugarForm.dataset.editando = '';
  campo(lugarForm, 'id').readOnly = false;
  campo(lugarForm, 'id').value = siguienteId(mapa.lugares);
  $('#lugar-titulo').textContent = 'Registrar un lugar';
  $('#guardar-lugar').textContent = 'Registrar lugar';
}
function nuevoCamino() {
  caminoForm.reset();
  caminoForm.dataset.editando = '';
  campo(caminoForm, 'id').readOnly = false;
  campo(caminoForm, 'id').value = siguienteId(mapa.caminos);
  for (const nombre of ['origen', 'destino', 'bidireccional']) campo(caminoForm, nombre).disabled = false;
  $('#camino-titulo').textContent = 'Conectar dos lugares';
  $('#guardar-camino').textContent = 'Registrar camino';
  $('#camino-ayuda').textContent = 'Registra un solo camino por pareja de lugares. Las medidas deben verificarse localmente.';
}
function editarLugar(lugar) {
  if (ocupado) return;
  nuevoLugar();
  lugarForm.dataset.editando = String(lugar.id);
  for (const nombre of ['id', 'nombre', 'descripcion', 'categoria']) campo(lugarForm, nombre).value = lugar[nombre];
  campo(lugarForm, 'activo').checked = lugar.activo;
  campo(lugarForm, 'id').readOnly = true;
  $('#lugar-titulo').textContent = `Editar lugar #${lugar.id}`;
  $('#guardar-lugar').textContent = 'Guardar cambios del lugar';
  lugarForm.scrollIntoView({ behavior: 'auto', block: 'center' });
  campo(lugarForm, 'nombre').focus({ preventScroll: true });
}
function editarCamino(camino) {
  if (ocupado) return;
  nuevoCamino();
  caminoForm.dataset.editando = String(camino.id);
  for (const nombre of ['id', 'origen', 'destino', 'distancia', 'tiempo']) campo(caminoForm, nombre).value = camino[nombre];
  campo(caminoForm, 'nota').value = camino.nota || '';
  for (const nombre of ['bidireccional', 'habilitado']) campo(caminoForm, nombre).checked = camino[nombre];
  campo(caminoForm, 'id').readOnly = true;
  for (const nombre of ['origen', 'destino', 'bidireccional']) campo(caminoForm, nombre).disabled = true;
  $('#camino-titulo').textContent = `Editar camino #${camino.id}`;
  $('#guardar-camino').textContent = 'Guardar cambios del camino';
  $('#camino-ayuda').textContent = 'Los cambios afectan ambos sentidos si el camino es bidireccional. Para cambiar los extremos o el sentido, elimina y vuelve a crear el camino.';
  caminoForm.scrollIntoView({ behavior: 'auto', block: 'center' });
  campo(caminoForm, 'distancia').focus({ preventScroll: true });
}
function pedirEliminacion(tipo, id, nombre) {
  if (ocupado) return;
  eliminacion = { tipo, id };
  $('#eliminar-texto').textContent = tipo === 'lugares'
    ? `¿Eliminar «${nombre}» y todos sus caminos asociados?`
    : `¿Eliminar el camino #${id} en todos sus sentidos?`;
  $('#confirmacion').showModal();
}
function dibujarLugares() {
  const consulta = $('#buscar-lugar').value.trim().toLocaleLowerCase('es');
  const visibles = mapa.lugares.filter(l => `${l.id} ${l.nombre} ${l.categoria}`.toLocaleLowerCase('es').includes(consulta));
  const catalogo = $('#catalogo');
  catalogo.replaceChildren();
  if (!visibles.length) catalogo.append(nodo('p', mapa.lugares.length ? 'No hay coincidencias con tu búsqueda.' : 'Aún no hay lugares. Registra el primero con el formulario.', 'empty'));
  for (const l of visibles) {
    const tarjeta = nodo('article', undefined, 'item-card');
    const titulo = nodo('div', undefined, 'panel-heading');
    titulo.append(nodo('span', `${l.categoria} · #${l.id}`, 'item-meta'), nodo('span', l.activo ? 'Activo' : 'Inactivo', l.activo ? 'status-pill' : 'status-pill inactive'));
    tarjeta.append(titulo, nodo('h4', l.nombre), nodo('p', l.descripcion || 'Sin descripción.'));
    const botones = nodo('div', undefined, 'item-actions');
    botones.append(accion(`Editar lugar #${l.id}`, () => editarLugar(l)), accion(`Eliminar lugar #${l.id}`, () => pedirEliminacion('lugares', l.id, l.nombre), true));
    tarjeta.append(botones);
    catalogo.append(tarjeta);
  }
}
function dibujar() {
  actualizarBotonRuta();
  $('#contador').textContent = String(mapa.lugares.length);
  $('#contador-caminos').textContent = String(mapa.caminos.length);
  dibujarLugares();
  const lista = $('#lista-caminos');
  lista.replaceChildren();
  if (!mapa.caminos.length) lista.append(nodo('p', 'Todavía no hay caminos. Registra dos lugares para conectarlos.', 'empty'));
  const nombre = id => mapa.lugares.find(l => l.id === id)?.nombre || `#${id}`;
  for (const c of mapa.caminos) {
    const tarjeta = nodo('article', undefined, 'item-card');
    const titulo = nodo('div', undefined, 'panel-heading');
    titulo.append(nodo('span', `Camino #${c.id} · ${c.bidireccional ? 'Ambos sentidos' : 'Un solo sentido'}`, 'item-meta'), nodo('span', c.habilitado ? 'Habilitado' : 'Cerrado', c.habilitado ? 'status-pill' : 'status-pill inactive'));
    tarjeta.append(titulo, nodo('h4', `${nombre(c.origen)} ${c.bidireccional ? '↔' : '→'} ${nombre(c.destino)}`), nodo('p', `${c.distancia.toLocaleString('es-EC')} metros · ${c.tiempo.toLocaleString('es-EC')} minutos`));
    if (c.nota) tarjeta.append(nodo('p', c.nota, 'help'));
    const botones = nodo('div', undefined, 'item-actions');
    botones.append(accion(`Editar camino #${c.id}`, () => editarCamino(c)), accion(`Eliminar camino #${c.id}`, () => pedirEliminacion('caminos', c.id), true));
    tarjeta.append(botones);
    lista.append(tarjeta);
  }
  for (const nombre of ['origen', 'destino']) opciones(campo(caminoForm, nombre), mapa.lugares);
  for (const selector of ['#origen', '#destino']) opciones($(selector), mapa.lugares.filter(l => l.activo));
  if (!lugarForm.dataset.editando && !lugarForm.contains(document.activeElement)) campo(lugarForm, 'id').value = siguienteId(mapa.lugares);
  if (!caminoForm.dataset.editando && !caminoForm.contains(document.activeElement)) campo(caminoForm, 'id').value = siguienteId(mapa.caminos);
  diagrama.actualizar(mapa, $('#origen').value, $('#destino').value);
}
async function actualizar() {
  const datos = await solicitar('/api/mapa');
  if (!Array.isArray(datos.lugares) || !Array.isArray(datos.caminos)) throw new Error('Respuesta del catálogo inesperada.');
  mapa = datos;
  $('#estado-guardado').textContent = mapa.persistencia === true
    ? 'Guardado automático activo. Los cambios confirmados se conservan al cerrar el programa.'
    : 'Esta versión del servidor todavía no guarda archivos. Reinicia el programa actualizado.';
  limpiarRuta('Elige origen, destino y criterio para calcular un recorrido.');
  dibujar();
  $('#estado').textContent = 'Conexión correcta · Lugares y caminos administrados por C++.';
}
async function guardarOperacion(url, metodo, datos, alCompletar) {
  if (ocupado) return;
  ocupado = true;
  document.body.setAttribute('aria-busy', 'true');
  for (const id of ['guardar-lugar', 'guardar-camino', 'reintentar', 'buscar-ruta', 'guardar-mapa', 'cargar-mapa']) $(`#${id}`).disabled = true;
  let modificacionConfirmada = false;
  try {
    const resultado = await solicitar(url, metodo, datos);
    modificacionConfirmada = true;
    limpiarRuta('El mapa cambió. Vuelve a calcular tu recorrido.');
    await actualizar();
    if (alCompletar) alCompletar();
    mensaje(resultado.mensaje);
  } catch (error) {
    mensaje(modificacionConfirmada ? 'El cambio se realizó, pero no se pudo actualizar el catálogo. Pulsa Actualizar catálogo antes de continuar.' : error.message, true);
  } finally {
    ocupado = false;
    document.body.removeAttribute('aria-busy');
    for (const id of ['guardar-lugar', 'guardar-camino', 'reintentar', 'buscar-ruta', 'guardar-mapa', 'cargar-mapa']) $(`#${id}`).disabled = false;
    actualizarBotonRuta();
  }
}
lugarForm.addEventListener('submit', event => {
  event.preventDefault();
  const datos = Object.fromEntries(new FormData(lugarForm));
  datos.nombre = datos.nombre.trim();
  datos.categoria = datos.categoria.trim();
  datos.activo = String(campo(lugarForm, 'activo').checked);
  const id = lugarForm.dataset.editando;
  guardarOperacion(id ? `/api/lugares/${id}` : '/api/lugares', id ? 'PUT' : 'POST', datos, nuevoLugar);
});
caminoForm.addEventListener('submit', event => {
  event.preventDefault();
  const datos = Object.fromEntries(new FormData(caminoForm));
  datos.bidireccional = String(campo(caminoForm, 'bidireccional').checked);
  datos.habilitado = String(campo(caminoForm, 'habilitado').checked);
  const id = caminoForm.dataset.editando;
  guardarOperacion(id ? `/api/caminos/${id}` : '/api/caminos', id ? 'PUT' : 'POST', datos, nuevoCamino);
});
$('#cancelar-lugar').addEventListener('click', () => { if (!ocupado) nuevoLugar(); });
$('#cancelar-camino').addEventListener('click', () => { if (!ocupado) nuevoCamino(); });
$('#buscar-lugar').addEventListener('input', dibujarLugares);
$('#cancelar-eliminar').addEventListener('click', () => $('#confirmacion').close());
$('#confirmar-eliminar').addEventListener('click', () => {
  $('#confirmacion').close();
  if (!eliminacion) return;
  const { tipo, id } = eliminacion;
  eliminacion = null;
  guardarOperacion(`/api/${tipo}/${id}`, 'DELETE', undefined, () => { nuevoLugar(); nuevoCamino(); });
});
$('#reintentar').addEventListener('click', async () => {
  if (ocupado) return;
  ocupado = true;
  $('#reintentar').disabled = true;
  try { await actualizar(); mensaje('Catálogo actualizado.'); }
  catch (error) { mensaje(error.message, true); $('#estado').textContent = 'Sin conexión con el programa de C++.'; }
  finally { ocupado = false; actualizarBotonRuta(); $('#reintentar').disabled = false; }
});
function actualizarBotonRuta() {
  for (const id of ['guardar-mapa', 'cargar-mapa']) $(`#${id}`).disabled = ocupado || mapa.persistencia !== true;
  $('#buscar-ruta').disabled = ocupado || !mapa.lugares.some(lugar => lugar.activo);
}
function limpiarRuta(texto) {
  diagrama.seleccionarExtremos($('#origen').value, $('#destino').value);
  diagrama.resaltarRuta(null);
  ++revisionRuta;
  const salida = $('#resultado-ruta');
  salida.classList.remove('route-error');
  salida.replaceChildren(nodo('p', texto));
}
function mostrarRuta(ruta) {
  diagrama.resaltarRuta(ruta);
  const salida = $('#resultado-ruta');
  salida.replaceChildren();
  if (!ruta.encontrada) {
    salida.append(nodo('h3', 'No hay un recorrido disponible'), nodo('p', 'Los puntos elegidos no están conectados por caminos habilitados en ese sentido. Revisa las conexiones y los lugares activos.'));
    return;
  }
  const formato = valor => valor.toLocaleString('es-EC', { maximumFractionDigits: 3 });
  salida.append(nodo('h3', ruta.criterio === 'distancia' ? 'Ruta de menor distancia' : 'Ruta de menor tiempo'));
  const verEsquema = nodo('a', 'Ver recorrido en el esquema →', 'route-map-link');
  verEsquema.href = '#mapa-visual';
  salida.append(verEsquema);
  const totales = nodo('div', undefined, 'route-totals');
  totales.append(nodo('p', `${formato(ruta.distanciaTotal)} metros`, 'route-metric'), nodo('p', `${formato(ruta.tiempoTotal)} minutos estimados`, 'route-metric'));
  salida.append(totales);
  const notas = [...new Set(ruta.tramos.map(tramo => tramo.nota).filter(Boolean))];
  if (notas.length) {
    salida.append(nodo('p', 'Medidas aproximadas: revisa las notas de los tramos. El total suma las estimaciones registradas y no incluye las visitas.', 'help'));
  }
  const secuencia = nodo('ol', undefined, 'route-stops');
  for (const lugar of ruta.lugares) secuencia.append(nodo('li', lugar.nombre));
  salida.append(secuencia);
  if (ruta.tramos.length === 0) {
    salida.append(nodo('p', 'El origen y el destino son el mismo lugar. No necesitas desplazarte.'));
  } else {
    const detalle = nodo('details');
    detalle.append(nodo('summary', `Ver detalle de ${ruta.tramos.length} ${ruta.tramos.length === 1 ? 'tramo' : 'tramos'}`));
    const lista = nodo('ol', undefined, 'route-details');
    const nombres = new Map(ruta.lugares.map(lugar => [lugar.id, lugar.nombre]));
    for (const tramo of ruta.tramos) lista.append(nodo('li', `${nombres.get(tramo.origen)} → ${nombres.get(tramo.destino)}: ${formato(tramo.distancia)} m · ${formato(tramo.tiempo)} min${tramo.nota ? ' · ' + tramo.nota : ''}`));
    detalle.append(lista);
    salida.append(detalle);
  }
}
rutaForm.addEventListener('change', () => limpiarRuta('La selección cambió. Pulsa Buscar recorrido para actualizar el resultado.'));
rutaForm.addEventListener('submit', async event => {
  event.preventDefault();
  if (ocupado) return;
  ocupado = true;
  const botones = ['guardar-lugar', 'guardar-camino', 'reintentar', 'buscar-ruta', 'guardar-mapa', 'cargar-mapa'];
  for (const id of botones) $(`#${id}`).disabled = true;
  $('#buscar-ruta').textContent = 'Calculando recorrido…';
  limpiarRuta('Buscando un recorrido disponible…');
  const revision = revisionRuta;
  try {
    const query = new URLSearchParams(new FormData(rutaForm));
    const ruta = await solicitar(`/api/ruta?${query}`);
    if (revision === revisionRuta) mostrarRuta(ruta);
  } catch (error) {
    if (revision === revisionRuta) {
      $('#resultado-ruta').classList.add('route-error');
      $('#resultado-ruta').replaceChildren(nodo('p', error.message));
    }
  } finally {
    ocupado = false;
    for (const id of botones) $(`#${id}`).disabled = false;
    actualizarBotonRuta();
    $('#buscar-ruta').textContent = 'Buscar recorrido';
  }
});
$('#guardar-mapa').addEventListener('click', () => {
  guardarOperacion('/api/guardar', 'POST', {});
});
$('#cargar-mapa').addEventListener('click', () => {
  if (!ocupado && mapa.persistencia === true) $('#confirmacion-carga').showModal();
});
$('#cancelar-carga').addEventListener('click', () => $('#confirmacion-carga').close());
$('#confirmar-carga').addEventListener('click', () => {
  $('#confirmacion-carga').close();
  guardarOperacion('/api/cargar', 'POST', {}, () => {
    nuevoLugar();
    nuevoCamino();
    $('#buscar-lugar').value = '';
    dibujarLugares();
  });
});
nuevoLugar();
nuevoCamino();
actualizar().catch(error => { mensaje(error.message, true); $('#estado').textContent = 'Inicia el programa de C++ para cargar el catálogo.'; });





