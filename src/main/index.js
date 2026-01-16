const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const native = require('./native');
const context = require('./context');

ipcMain.handle('context:get', (event, key) => {
  return context.get(key);
});

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      preload: path.join(__dirname, '../preload/index.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    }
  });

  context.addListener(mainWindow);

  mainWindow.loadFile(path.join(__dirname, '../renderer/index.html'));

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
