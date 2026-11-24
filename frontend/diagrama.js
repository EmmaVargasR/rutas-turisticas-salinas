// El dibujo no calcula rutas ni distancias: presenta los datos del motor C++.
export function distribuirLugares(lugares) {
  const ordenados = [...lugares].sort((a, b) => a.id - b.id);
  return ordenados.map((lugar, indice) => {
    const angulo = -Math.PI / 2 + 2 * Math.PI * indice / ordenados.length;
    return { ...lugar, x: ordenados.length === 1 ? 480 : 480 + 300 * Math.cos(angulo), y: ordenados.length === 1 ? 300 : 285 + 190 * Math.sin(angulo) };
  });
}
export function prepararDiagrama(mapa, ruta = null) {
  const lugares = distribuirLugares(mapa.lugares);
  const porId = new Map(lugares.map(lugar => [lugar.id, lugar]));
  const paradas = new Set(ruta?.encontrada ? ruta.lugares.map(lugar => lugar.id) : []);
  const caminos = mapa.caminos.flatMap(camino => {
    const origen = porId.get(camino.origen), destino = porId.get(camino.destino);
    if (!origen || !destino || origen.id === destino.id) return [];
    const disponible = camino.habilitado && origen.activo && destino.activo;
    const tramo = disponible && ruta?.encontrada ? ruta.tramos.find(t => t.idCamino === camino.id && (
      (t.origen === origen.id && t.destino === destino.id) ||
      (camino.bidireccional && t.origen === destino.id && t.destino === origen.id)
    )) : null;
    const dx = destino.x - origen.x, dy = destino.y - origen.y;
    const longitud = Math.hypot(dx, dy), ux = dx / longitud, uy = dy / longitud;
    const inicio = { x: origen.x + ux * 36, y: origen.y + uy * 36 };
    const fin = { x: destino.x - ux * 36, y: destino.y - uy * 36 };
    const control = { x: (inicio.x + fin.x) / 2 - uy * 22, y: (inicio.y + fin.y) / 2 + ux * 22 };
    return [{ ...camino, origenLugar: origen, destinoLugar: destino, disponible, enRuta: Boolean(tramo),
      sentidoRuta: tramo?.origen === destino.id ? 'inverso' : 'directo',
      dibujo: `M ${inicio.x} ${inicio.y} Q ${control.x} ${control.y} ${fin.x} ${fin.y}`,
      puntos: { inicio, control, fin }, etiqueta: { x: (inicio.x + 2 * control.x + fin.x) / 4, y: (inicio.y + 2 * control.y + fin.y) / 4 }
    }];
  });
  // Separar las etiquetas en los cruces, dando prioridad al recorrido resaltado.
  const ocupadas = [];
  for (const c of [...caminos].sort((a,b) => Number(b.enRuta)-Number(a.enRuta))) {
    const ancho = Math.max(100, `${c.distancia} m · ${c.tiempo} min`.length * 6.2 + 14);
    let elegida = c.etiqueta;
    let minimo = Infinity;
    for (const t of [.5,.35,.65,.25,.75]) {
      const { inicio:a, control:b, fin:d } = c.puntos;
      const punto = { x:(1-t)**2*a.x+2*(1-t)*t*b.x+t*t*d.x, y:(1-t)**2*a.y+2*(1-t)*t*b.y+t*t*d.y };
      const caja = { x:punto.x-ancho/2-4,y:punto.y-16,w:ancho+8,h:32 };
      const choques = ocupadas.filter(o=>caja.x<o.x+o.w && caja.x+caja.w>o.x && caja.y<o.y+o.h && caja.y+caja.h>o.y).length
        + lugares.filter(l=>
          (caja.x<l.x+33 && caja.x+caja.w>l.x-33 && caja.y<l.y+33 && caja.y+caja.h>l.y-33) ||
          (caja.x<l.x+98 && caja.x+caja.w>l.x-98 && caja.y<l.y+78 && caja.y+caja.h>l.y+32)
        ).length;
      if (choques < minimo) { minimo=choques; elegida=punto; }
      if (!choques) break;
    }
    c.etiqueta = elegida;
    ocupadas.push({x:elegida.x-ancho/2-4,y:elegida.y-16,w:ancho+8,h:32});
  }
  return { lugares: lugares.map(lugar => ({ ...lugar, enRuta: lugar.activo && paradas.has(lugar.id) })), caminos };
}
function svgNodo(nombre, atributos = {}, contenido) {
  const elemento = document.createElementNS('http://www.w3.org/2000/svg', nombre);
  for (const [clave, valor] of Object.entries(atributos)) elemento.setAttribute(clave, valor);
  if (contenido !== undefined) elemento.textContent = contenido;
  return elemento;
}
function textoHtml(nombre, texto, clase) {
  const elemento = document.createElement(nombre);
  if (texto !== undefined) elemento.textContent = texto;
  if (clase) elemento.className = clase;
  return elemento;
}
function lineasNombre(nombre) {
  const lineas = [];
  let resto = nombre.trim();
  while (resto && lineas.length < 2) {
    if (resto.length <= 24) { lineas.push(resto); resto=''; break; }
    const espacio = resto.lastIndexOf(' ',24);
    const corte = espacio>0 ? espacio : 24;
    lineas.push(resto.slice(0,corte));
    resto=resto.slice(corte).trim();
  }
  if (resto) lineas[lineas.length-1]=lineas.at(-1).slice(0,21)+'…';
  return lineas.length ? lineas : ['Sin nombre'];
}
export class DiagramaMapa {
  constructor(contenedor, alElegirExtremo) {
    this.contenedor = contenedor;
    this.alElegirExtremo = alElegirExtremo;
    this.mapa = { lugares: [], caminos: [] };
    this.ruta = null;
    this.seleccionado = null;
    this.extremos = { origen: null, destino: null };
    this.zoom = 1;
    this.lienzo = contenedor.querySelector('#diagrama-lienzo');
    this.detalle = contenedor.querySelector('#diagrama-detalle');
    contenedor.querySelector('#diagrama-acercar').addEventListener('click', () => this.cambiarZoom(.25));
    contenedor.querySelector('#diagrama-alejar').addEventListener('click', () => this.cambiarZoom(-.25));
    contenedor.querySelector('#diagrama-restablecer').addEventListener('click', () => {
      this.zoom = 1; this.aplicarZoom(); this.lienzo.scrollTo({ left: 0, top: 0 });
    });
    this.observador = new ResizeObserver(() => this.aplicarZoom());
    this.observador.observe(this.lienzo);
    this.renderizar();
  }
  actualizar(mapa, origen, destino) {
    this.mapa = mapa; this.ruta = null; this.extremos = { origen: Number(origen) || null, destino: Number(destino) || null };
    if (!mapa.lugares.some(lugar => lugar.id === this.seleccionado)) this.seleccionado = null;
    this.renderizar();
  }
  resaltarRuta(ruta) { this.ruta = ruta?.encontrada ? ruta : null; this.renderizar(); }
  seleccionarExtremos(origen, destino) {
    this.extremos = { origen: Number(origen) || null, destino: Number(destino) || null };
    this.renderizar();
  }
  cambiarZoom(cambio) { this.zoom = Math.max(1, Math.min(2, this.zoom + cambio)); this.aplicarZoom(); }
  aplicarZoom() {
    const svg = this.lienzo.querySelector('svg');
    if (svg) svg.style.width = `${Math.max(760, this.lienzo.clientWidth) * this.zoom}px`;
    const vacio = !this.mapa.lugares.length;
    this.contenedor.querySelector('#diagrama-zoom').textContent = `${Math.round(this.zoom * 100)} %`;
    this.contenedor.querySelector('#diagrama-acercar').disabled = vacio || this.zoom >= 2;
    this.contenedor.querySelector('#diagrama-alejar').disabled = vacio || this.zoom <= 1;
    this.contenedor.querySelector('#diagrama-restablecer').disabled = vacio;
  }
  elegirLugar(id) { this.seleccionado = id; this.renderizar(); }
  renderizar() {
    const foco = document.activeElement;
    const focoLugar = foco?.getAttribute('data-lugar');
    const focoExtremo = this.detalle.contains(foco) ? foco?.getAttribute('data-extremo') : null;
    const modelo = prepararDiagrama(this.mapa, this.ruta);
    this.contenedor.querySelector('#diagrama-conteo').textContent = `${modelo.lugares.length} ${modelo.lugares.length === 1 ? 'lugar' : 'lugares'} · ${modelo.caminos.length} ${modelo.caminos.length === 1 ? 'camino' : 'caminos'}`;
    this.contenedor.querySelector('#diagrama-ruta-estado').textContent = this.ruta
      ? `Resaltado: recorrido de menor ${this.ruta.criterio === 'tiempo' ? 'tiempo' : 'distancia'}.`
      : 'Calcula un recorrido para resaltarlo en el esquema.';
    this.lienzo.replaceChildren();
    if (!modelo.lugares.length) {
      this.lienzo.append(textoHtml('p', 'Registra tu primer lugar para verlo en el esquema.', 'empty'));
      this.mostrarDetalle(); this.aplicarZoom(); return;
    }
    const svg = svgNodo('svg', { viewBox: '0 0 960 600', role: 'group', 'aria-label': 'Esquema interactivo de lugares y caminos', class: 'map-svg' });
    const defs = svgNodo('defs');
    for (const [id, color] of [['normal', '#63816c'], ['cerrado', '#aaa89e'], ['ruta', '#a96817']]) {
      const marcador = svgNodo('marker', { id: `flecha-${id}`, viewBox: '0 0 10 10', refX: 9, refY: 5, markerWidth: 10, markerHeight: 10, orient: 'auto-start-reverse', markerUnits: 'userSpaceOnUse' });
      marcador.append(svgNodo('path', { d: 'M 0 0 L 10 5 L 0 10 z', fill: color })); defs.append(marcador);
    }
    svg.append(defs, svgNodo('text', { x: 26, y: 32, class: 'map-caption' }, 'SALINAS · ESQUEMA DE CONEXIONES'));
    const etiquetas = [];
    for (const c of [...modelo.caminos].sort((a, b) => Number(a.enRuta) - Number(b.enRuta))) {
      const estado = c.disponible ? 'normal' : 'cerrado';
      const grupo = svgNodo('g', { class: `map-edge ${c.disponible ? '' : 'unavailable'} ${c.enRuta ? 'highlighted' : ''}`, 'data-camino': c.id, 'data-en-ruta': c.enRuta, role: 'img', 'aria-label': `${c.origenLugar.nombre} ${c.bidireccional ? 'en ambos sentidos con' : 'hacia'} ${c.destinoLugar.nombre}, ${c.distancia} metros, ${c.tiempo} minutos${c.disponible ? '' : ', no disponible para rutas'}` });
      grupo.append(svgNodo('title', {}, `Camino #${c.id}: ${c.origenLugar.nombre} ${c.bidireccional ? '↔' : '→'} ${c.destinoLugar.nombre}`));
      const base = svgNodo('path', { d: c.dibujo, fill: 'none', 'marker-end': `url(#flecha-${estado})` });
      if (c.bidireccional) base.setAttribute('marker-start', `url(#flecha-${estado})`);
      grupo.append(base);
      if (c.enRuta) grupo.append(svgNodo('path', { d: c.dibujo, fill: 'none', class: 'map-route-edge', [c.sentidoRuta === 'inverso' ? 'marker-start' : 'marker-end']: 'url(#flecha-ruta)' }));
      const etiqueta = `${c.distancia.toLocaleString('es-EC', { maximumFractionDigits: 2 })} m · ${c.tiempo.toLocaleString('es-EC', { maximumFractionDigits: 2 })} min`;
      const ancho = Math.max(100, etiqueta.length * 6.2 + 14);
      const rotulo = svgNodo('g', { class: `map-edge ${c.disponible ? '' : 'unavailable'} ${c.enRuta ? 'highlighted' : ''}`, 'aria-hidden': 'true' });
      rotulo.append(svgNodo('rect', { x: c.etiqueta.x - ancho / 2, y: c.etiqueta.y - 12, width: ancho, height: 24, rx: 8, class: 'map-edge-label-bg' }), svgNodo('text', { x: c.etiqueta.x, y: c.etiqueta.y + 4, 'text-anchor': 'middle', class: 'map-edge-label' }, etiqueta));
      etiquetas.push(rotulo);
      svg.append(grupo);
    }
    svg.append(...etiquetas);
    const gruposLugar = new Map();
    for (const l of modelo.lugares) {
      const esOrigen = l.activo && this.extremos.origen === l.id, esDestino = l.activo && this.extremos.destino === l.id;
      const g = svgNodo('g', { transform: `translate(${l.x}, ${l.y})`, class: `map-node ${l.activo ? '' : 'inactive'} ${l.enRuta ? 'highlighted' : ''} ${this.seleccionado === l.id ? 'selected' : ''}`, role: 'button', tabindex: 0, 'data-lugar': l.id, 'data-en-ruta': l.enRuta, 'aria-label': `Ver lugar ${l.nombre}, ID ${l.id}${l.activo ? '' : ', inactivo'}`, 'aria-pressed': this.seleccionado === l.id });
      g.append(svgNodo('title', {}, l.nombre), svgNodo('circle', { r: 29, class: 'map-node-halo' }), svgNodo('circle', { r: 25, class: 'map-node-dot' }));
      g.append(svgNodo('text', { 'text-anchor': 'middle', y: 4, class: 'map-node-id', 'font-size': String(l.id).length > 5 ? 8 : 12 }, `#${l.id}`));
      if (esOrigen || esDestino) g.append(svgNodo('text', { y: -39, 'text-anchor': 'middle', class: 'map-extremo' }, esOrigen && esDestino ? 'ORIGEN Y DESTINO' : esOrigen ? 'ORIGEN' : 'DESTINO'));
      const lineas = lineasNombre(l.nombre);
      g.append(svgNodo('rect', { x: -96, y: 34, width: 192, height: lineas.length * 16 + 10, rx: 5, class: 'map-node-label-bg' }));
      const nombre = svgNodo('text', { 'text-anchor': 'middle', class: 'map-node-name' });
      lineas.forEach((linea, i) => nombre.append(svgNodo('tspan', { x: 0, y: 49 + i * 16 }, linea)));
      g.append(nombre);
      g.addEventListener('click', () => this.elegirLugar(l.id));
      g.addEventListener('keydown', event => { if (event.key === 'Enter' || event.key === ' ') { event.preventDefault(); this.elegirLugar(l.id); } });
      gruposLugar.set(l.id, g); svg.append(g);
    }
    this.lienzo.append(svg); this.mostrarDetalle(); this.aplicarZoom();
    if (focoLugar && gruposLugar.has(Number(focoLugar))) gruposLugar.get(Number(focoLugar)).focus({ preventScroll: true });
    if (focoExtremo) this.detalle.querySelector(`[data-extremo="${focoExtremo}"]`)?.focus({ preventScroll: true });
  }
  mostrarDetalle() {
    const lugar = this.mapa.lugares.find(l => l.id === this.seleccionado);
    this.detalle.replaceChildren();
    if (!lugar) { this.detalle.append(textoHtml('h3', 'Conoce cada lugar'), textoHtml('p', 'Selecciona un punto del esquema para ver su descripción y elegirlo como origen o destino.')); return; }
    this.detalle.append(textoHtml('span', `${lugar.categoria} · #${lugar.id}`, 'item-meta'), textoHtml('h3', lugar.nombre), textoHtml('p', lugar.descripcion || 'Sin descripción.'), textoHtml('p', lugar.activo ? 'Lugar activo' : 'Lugar inactivo · No participa en los recorridos', 'help'));
    const acciones = textoHtml('div', undefined, 'form-actions');
    for (const [extremo, etiqueta] of [['origen', 'Usar como origen'], ['destino', 'Usar como destino']]) {
      const boton = textoHtml('button', etiqueta, extremo === 'origen' ? 'primary' : 'secondary');
      boton.type = 'button'; boton.dataset.extremo = extremo; boton.disabled = !lugar.activo;
      boton.addEventListener('click', () => this.alElegirExtremo(lugar, extremo)); acciones.append(boton);
    }
    this.detalle.append(acciones);
    const salientes = this.mapa.caminos.filter(c => c.origen === lugar.id || (c.bidireccional && c.destino === lugar.id));
    this.detalle.append(textoHtml('h4', `Conexiones salientes (${salientes.length})`));
    const lista = textoHtml('ul', undefined, 'map-connections');
    for (const c of salientes) {
      const destino = this.mapa.lugares.find(l => l.id === (c.origen === lugar.id ? c.destino : c.origen));
      lista.append(textoHtml('li', `${destino?.nombre || '?'} · ${c.distancia} m · ${c.tiempo} min${c.habilitado && lugar.activo && destino?.activo ? '' : ' · No disponible'}${c.nota ? ' · ' + c.nota : ''}`));
    }
    if (!salientes.length) lista.append(textoHtml('li', 'Sin caminos salientes registrados.'));
    this.detalle.append(lista);
  }
}





