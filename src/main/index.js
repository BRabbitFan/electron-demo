import {app, BrowserWindow, ipcMain} from 'electron';
import {join} from 'path';

import context from '#main/context.js';
import native from '#main/native.js';

import.meta.resolve = function(modulePath) {
  return new URL(modulePath, import.meta.url).pathname;
}

ipcMain.handle('context:get', (event, key) => {
  return context.get(key);
});

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      preload: join(import.meta.dirname, '../preload/index.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    }
  });

  context.addListener(mainWindow);

  mainWindow.loadFile(join(import.meta.dirname, '../renderer/index.html'));

  mainWindow.webContents.on('did-finish-load', () => {
    mainWindow.webContents.send('context:state-changed', context.getState());
  });

  mainWindow.on('closed', () => {
    context.removeListener(mainWindow);
  });

  return mainWindow;
}

app.whenReady().then(() => {
  context.setState({
    data: native.addon.get_message(),
  });

  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
