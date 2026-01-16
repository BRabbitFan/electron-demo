
let element = document.getElementById('message');

element.textContent = window.context.get('data');

window.context.onStateChanged((state) => {
  element.textContent = state.data;
});
