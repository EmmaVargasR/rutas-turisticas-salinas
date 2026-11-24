// Fotografías locales: ampliar no modifica el catálogo ni el recorrido calculado.
const dialogo = document.querySelector('#foto-ampliada');
const imagen = document.querySelector('#foto-imagen');
const titulo = document.querySelector('#foto-titulo');

for (const boton of document.querySelectorAll('[data-foto]')) {
  boton.addEventListener('click', () => {
    imagen.src = boton.dataset.foto;
    imagen.alt = boton.dataset.alt;
    titulo.textContent = boton.dataset.titulo;
    dialogo.showModal();
  });
}
document.querySelector('#cerrar-foto').addEventListener('click', () => dialogo.close());
dialogo.addEventListener('click', event => {
  if (event.target !== dialogo) return;
  const area = dialogo.getBoundingClientRect();
  if (event.clientX < area.left || event.clientX > area.right ||
      event.clientY < area.top || event.clientY > area.bottom) dialogo.close();
});
