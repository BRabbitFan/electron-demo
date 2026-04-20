import { app, BrowserWindow, ipcMain } from 'electron';
import { createRequire } from 'module';
import { join } from 'path';

import { nodeApis } from './node-apis.js';

const require = createRequire(import.meta.url);
const nativeApis = require(join(app.getAppPath(), 'build/bin/brabbit_native_apis.node'));

nativeApis.SetNodeApis(...nodeApis);

function setupIpcHandlers() {
  // GL offscreen rendering with PBO readback.
  ipcMain.handle('gl:create', (_event, width, height) => {
    return nativeApis.CreateGlRenderer(width, height);
  });

  ipcMain.handle('gl:render', () => {
    const pixels = nativeApis.RenderGl();
    if (!pixels) return null;
    const size = nativeApis.GetGlRendererSize();
    return { width: size.width, height: size.height, pixels };
  });

  ipcMain.handle('gl:resize', (_event, width, height) => {
    nativeApis.ResizeGlRenderer(width, height);
  });

  ipcMain.handle('gl:destroy', () => {
    nativeApis.DestroyGlRenderer();
  });

  ipcMain.handle('gl:rotate', (_event, dx, dy) => {
    nativeApis.RotateGlRenderer(dx, dy);
  });
}

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

  mainWindow.on('closed', () => {
    nativeApis.DestroyGlRenderer();
  });

  return mainWindow;
}

app.whenReady().then(() => {
  setupIpcHandlers();
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
