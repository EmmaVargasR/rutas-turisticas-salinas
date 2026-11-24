import test from 'node:test';
import assert from 'node:assert/strict';
import { distribuirLugares, prepararDiagrama } from '../frontend/diagrama.js';
const lugares = [{id:10,nombre:'A',activo:true},{id:20,nombre:'B',activo:true},{id:30,nombre:'C',activo:false}];
const camino = {id:7,origen:10,destino:20,distancia:100,tiempo:2,bidireccional:true,habilitado:true};
const ruta = (origen=20,destino=10) => ({encontrada:true,criterio:'distancia',lugares:[{id:origen},{id:destino}],tramos:[{idCamino:7,origen,destino}]});
test('vacio, un lugar y posiciones finitas, estables e independientes del orden de entrada', () => {
  assert.deepEqual(prepararDiagrama({lugares:[],caminos:[]}), {lugares:[],caminos:[]});
  const unico = distribuirLugares([lugares[0]])[0];
  assert.equal(unico.x,480); assert.equal(unico.y,300);
  const antes = JSON.stringify(lugares);
  assert.deepEqual(distribuirLugares(lugares),distribuirLugares([...lugares].reverse()));
  for (const punto of distribuirLugares(lugares)) {
    assert.ok(Number.isFinite(punto.x) && Number.isFinite(punto.y));
    assert.ok(punto.x>30 && punto.x<930 && punto.y>30 && punto.y<570);
  }
  assert.equal(JSON.stringify(lugares),antes);
});
test('un camino bidireccional se resalta tambien al recorrerlo al reves', () => {
  const resultado = prepararDiagrama({lugares,caminos:[camino]},ruta());
  assert.equal(resultado.caminos[0].enRuta,true);
  assert.equal(resultado.caminos[0].sentidoRuta,'inverso');
  assert.equal(resultado.lugares.filter(l=>l.enRuta).length,2);
});
test('no se resalta un sentido prohibido, un camino cerrado o un extremo inactivo', () => {
  for (const variante of [{...camino,bidireccional:false},{...camino,habilitado:false},{...camino,destino:30}]) {
    const modelo = prepararDiagrama({lugares,caminos:[variante]},ruta());
    assert.equal(modelo.caminos[0].enRuta,false);
  }
  const otroId = prepararDiagrama({lugares,caminos:[{...camino,id:8}]},ruta());
  assert.equal(otroId.caminos[0].enRuta,false);
});
test('cambiar o retirar la ruta elimina resaltados; origen igual a destino solo marca un lugar', () => {
  const mapa = {lugares,caminos:[camino]};
  prepararDiagrama(mapa,ruta());
  const limpio = prepararDiagrama(mapa,null);
  assert.ok(limpio.lugares.every(l=>!l.enRuta) && limpio.caminos.every(c=>!c.enRuta));
  const imposible = prepararDiagrama(mapa,{encontrada:false,lugares:[],tramos:[]});
  assert.ok(imposible.caminos.every(c=>!c.enRuta));
  const igual = prepararDiagrama(mapa,{encontrada:true,lugares:[{id:10}],tramos:[]});
  assert.deepEqual(igual.lugares.filter(l=>l.enRuta).map(l=>l.id),[10]);
  assert.ok(igual.caminos.every(c=>!c.enRuta));
});
test('referencias ausentes no producen lineas con coordenadas invalidas', () => {
  const modelo = prepararDiagrama({lugares,caminos:[{...camino,destino:999}]});
  assert.equal(modelo.caminos.length,0);
});
