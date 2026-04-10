const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  onMessage: (callback) => {
    ipcRenderer.on('native:message', (_, message) => callback(message));
  },
});
