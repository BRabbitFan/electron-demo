const messageElement = document.getElementById('message');
const native = window.nativeApis;

messageElement.textContent = 'Native API Version: ' + native.GetNativeApiVersion();

// Display native GL offscreen render via Canvas 2D.
const canvas = document.getElementById('glcanvas');
const ctx = canvas.getContext('2d');

if (!ctx) {
  messageElement.textContent = 'Canvas 2D not supported';
} else {
  const RENDER_WIDTH = 1920;
  const RENDER_HEIGHT = 1080;

  // --- Mouse drag rotation ---
  let isDragging = false;
  let lastX = 0, lastY = 0;

  canvas.addEventListener('mousedown', (e) => {
    isDragging = true;
    lastX = e.clientX;
    lastY = e.clientY;
  });
  window.addEventListener('mouseup', () => { isDragging = false; });
  window.addEventListener('mousemove', (e) => {
    if (!isDragging) return;
    const dx = e.clientX - lastX;
    const dy = e.clientY - lastY;
    lastX = e.clientX;
    lastY = e.clientY;
    native.RotateGlRenderer(dx, dy);
  });

  // Create the native GL renderer at fixed resolution.
  if (!native.CreateGlRenderer(RENDER_WIDTH, RENDER_HEIGHT)) {
    messageElement.textContent = 'Failed to create native GL renderer';
  } else {
    let frameCount = 0;
    let lastFpsTime = performance.now();
    let fps = 0;

    function renderLoop() {
      const pixelsBuf = native.RenderGl();
      if (pixelsBuf) {
        const size = native.GetGlRendererSize();
        const fw = size.width;
        const fh = size.height;

        const dpr = window.devicePixelRatio || 1;
        const cw = Math.round(canvas.clientWidth * dpr);
        const ch = Math.round(canvas.clientHeight * dpr);
        canvas.width = cw;
        canvas.height = ch;

        const imageData = new ImageData(new Uint8ClampedArray(pixelsBuf), fw, fh);
        createImageBitmap(imageData, { imageOrientation: 'flipY' }).then((bitmap) => {
          const frameAspect = fw / fh;
          const canvasAspect = cw / ch;
          let sx, sy, sw, sh;
          if (canvasAspect > frameAspect) {
            sw = fw; sh = fw / canvasAspect; sx = 0; sy = (fh - sh) / 2;
          } else {
            sh = fh; sw = fh * canvasAspect; sx = (fw - sw) / 2; sy = 0;
          }
          ctx.drawImage(bitmap, sx, sy, sw, sh, 0, 0, cw, ch);
          bitmap.close();

          frameCount++;
          const now = performance.now();
          if (now - lastFpsTime >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastFpsTime = now;
          }
          ctx.font = '28px monospace';
          ctx.fillStyle = 'lime';
          ctx.fillText(`${fps} FPS`, 12, 36);

          requestAnimationFrame(renderLoop);
        });
        return;
      }
      requestAnimationFrame(renderLoop);
    }

    requestAnimationFrame(renderLoop);
  }
}
