import {app} from 'electron';
import {createRequire} from 'module';
import {join} from 'path';

// ES modules 中使用 require
const require = createRequire(import.meta.url);

class Native {
  constructor() {
    this.addon = require(join(app.getAppPath(), 'build/Release/addon.node'));
  }
};

export default new Native();
