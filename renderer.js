// Do not remove the worker! It doesn't work without it
var worker = new Worker('./worker.js');
worker.onmessage = function(event) { 
}

let floatingWindow = window.open(`file://${__dirname}/floating.html`);

//address of native addon 
const {init, destroy, onFocus, setText} = require('./Nodejs-Napi-Addon-Using-Cmake/build/Release/addon.node'); 

console.log('Init: ', init());

setTimeout(() => {
   const focusOk = onFocus((l, t, r, b, controlType) => {
      if (floatingWindow) {
         if (l === 0 && t === 0 && r === 0 && b === 0) {
            floatingWindow.hide();
         } else {
            floatingWindow.moveTo(r-50,t);
            floatingWindow.show();
         }
      }

      console.log('Focus: ', l, t, r, b, controlType)
   });
   console.log('Focus event hadler installed: ', focusOk);
}, 1000);

function paste() {
   console.log(setText('Hello!'));
}

window.addEventListener('beforeunload', function(event) {
   alert('sss');
 })

