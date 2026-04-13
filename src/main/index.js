import { app, BrowserWindow } from 'electron';
import { createRequire } from 'module';
import { join } from 'path';

import { nodeApis } from './node-apis.js';

const require = createRequire(import.meta.url);
const nativeApis = require(join(app.getAppPath(), 'build/bin/native_apis.node'));

nativeApis.SetNodeApis(...nodeApis);

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      preload: join(import.meta.dirname, '../preload/index.cjs'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    }
  });

  mainWindow.setMenuBarVisibility(false);

  mainWindow.loadFile(join(import.meta.dirname, '../renderer/index.html'));

  if (app.isPackaged) {
    mainWindow.webContents.on('before-input-event', (event, input) => {
      // disable F12 and Ctrl+Shift+I to open DevTools in production
      if (input.key === 'F12' || (input.control && input.shift && input.key === 'I')) {
        event.preventDefault();
      }
    });
  }

  mainWindow.webContents.on('did-finish-load', () => {
    mainWindow.webContents.send('native:message', nativeApis.GetNativeApiVersion());
  });

  return mainWindow;
}

app.whenReady().then(() => {
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
