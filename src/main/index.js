import { app, BrowserWindow } from 'electron';
import { createRequire } from 'module';
import { join } from 'path';

const require = createRequire(import.meta.url);
const buildType = app.isPackaged || !process.env.DEBUG_NATIVE ? 'Release' : 'Debug';
const addon = require(join(app.getAppPath(), `build/${buildType}/addon.node`));

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

  mainWindow.webContents.on('did-finish-load', () => {
    mainWindow.webContents.send('native:message', addon.GetHelloWorld());
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
