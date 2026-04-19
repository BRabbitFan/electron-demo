const { join } = require('path');
const { contextBridge } = require('electron');

const nativeApis = require('bindings')('bin/brabbit_native_apis.node');
const { nodeApis } = require('./node-apis.cjs');

nativeApis.SetNodeApis(...nodeApis);

// Expose to isolated renderer context. Same OS process — no IPC.
contextBridge.exposeInMainWorld('nativeApis', {
  GetNativeApiVersion: () => nativeApis.GetNativeApiVersion(),
  CreateGlRenderer: (w, h) => nativeApis.CreateGlRenderer(w, h),
  DestroyGlRenderer: () => nativeApis.DestroyGlRenderer(),
  RenderGl: () => {
    const buf = nativeApis.RenderGl();
    if (!buf) return null;
    // Return a plain ArrayBuffer so contextBridge can structured-clone it (same-process memcpy).
    return buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
  },
  GetGlRendererSize: () => {
    const s = nativeApis.GetGlRendererSize();
    return { width: s.width, height: s.height };
  },
  ResizeGlRenderer: (w, h) => nativeApis.ResizeGlRenderer(w, h),
  RotateGlRenderer: (dx, dy) => nativeApis.RotateGlRenderer(dx, dy),
});
