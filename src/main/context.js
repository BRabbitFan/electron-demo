
class Context {
  constructor() {
    this.listeners = new Set();
    this.state = {
      data: String(),
    };
  }

  getState() {
    return { ...this.state };
  }

  get(key) {
    return this.state[key];
  }

  setState(updates) {
    this.state = {
      ...this.state,
      ...updates,
    };

    this.notifyListeners();
  }

  addListener(window) {
    this.listeners.add(window);
  }

  removeListener(window) {
    this.listeners.delete(window);
  }

  notifyListeners() {
    this.listeners.forEach(window => {
      if (!window.isDestroyed()) {
        window.webContents.send('context:state-changed', this.state);
      }
    });
  }
}

export default new Context();
