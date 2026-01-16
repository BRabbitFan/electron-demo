const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('context', {
  get: (key) => ipcRenderer.invoke('context:get', key),

  onStateChanged: (callback) => {
    ipcRenderer.on('context:state-changed', (_, state) => callback(state));
  },

  removeStateListener: () => {
    ipcRenderer.removeAllListeners('context:state-changed');
  }
});

console.log('[Preload] Context API exposed');
