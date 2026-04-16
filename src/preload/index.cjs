const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  onMessage: (callback) => {
    ipcRenderer.on('native:message', (_, message) => callback(message));
  },
  glCreate: (width, height) => ipcRenderer.invoke('gl:create', width, height),
  glRender: () => ipcRenderer.invoke('gl:render'),
  glResize: (width, height) => ipcRenderer.invoke('gl:resize', width, height),
  glRotate: (dx, dy) => ipcRenderer.invoke('gl:rotate', dx, dy),
  glDestroy: () => ipcRenderer.invoke('gl:destroy'),
});
