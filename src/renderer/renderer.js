const messageElement = document.getElementById('message');

window.api.onMessage((message) => {
  messageElement.textContent = "Native API Version: " + message;
});

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
    window.api.glRotate(dx, dy);
  });

  // Create the native GL renderer at fixed resolution.
  window.api.glCreate(RENDER_WIDTH, RENDER_HEIGHT).then((ok) => {
    if (!ok) {
      messageElement.textContent = 'Failed to create native GL renderer';
      return;
    }
    requestAnimationFrame(renderLoop);
  });

  let rendering = false;
  let frameCount = 0;
  let lastFpsTime = performance.now();
  let fps = 0;

  async function renderLoop() {
    if (!rendering) {
      rendering = true;
      try {
        const frame = await window.api.glRender();
        if (frame && frame.pixels) {
          const pixels = new Uint8ClampedArray(frame.pixels);
          const fw = frame.width;
          const fh = frame.height;

          // Match canvas pixel size to CSS size × devicePixelRatio
          const dpr = window.devicePixelRatio || 1;
          const cw = Math.round(canvas.clientWidth * dpr);
          const ch = Math.round(canvas.clientHeight * dpr);
          canvas.width = cw;
          canvas.height = ch;

          const imageData = new ImageData(pixels, fw, fh);
          const bitmap = await createImageBitmap(imageData, { imageOrientation: 'flipY' });

          // Cover-crop: scale to fill, center, clip excess
          const frameAspect = fw / fh;
          const canvasAspect = cw / ch;
          let sx, sy, sw, sh;
          if (canvasAspect > frameAspect) {
            // Canvas is wider → crop top/bottom of frame
            sw = fw;
            sh = fw / canvasAspect;
            sx = 0;
            sy = (fh - sh) / 2;
          } else {
            // Canvas is taller → crop left/right of frame
            sh = fh;
            sw = fh * canvasAspect;
            sx = (fw - sw) / 2;
            sy = 0;
          }
          ctx.drawImage(bitmap, sx, sy, sw, sh, 0, 0, cw, ch);
          bitmap.close();

          // FPS counter
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
        }
      } catch (e) {
        console.error('Render error:', e);
      }
      rendering = false;
    }
    requestAnimationFrame(renderLoop);
  }
}
