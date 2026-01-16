const { app } = require('electron');
const path = require('path');

class Native {
  constructor() {
    this.addon = require(path.join(app.getAppPath(), 'build/Release/addon.node'));
  };
};

module.exports = new Native();
