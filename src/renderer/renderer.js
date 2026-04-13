const messageElement = document.getElementById('message');

window.api.onMessage((message) => {
  messageElement.textContent = "Native API Version: " + message;
});

// WebGL
const canvas = document.getElementById('glcanvas');
const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');

if (!gl) {
  messageElement.textContent = 'WebGL not supported';
} else {
  const vsSource = `
    attribute vec4 aPosition;
    attribute vec3 aColor;
    varying vec3 vColor;
    void main() {
      gl_Position = aPosition;
      vColor = aColor;
    }
  `;

  const fsSource = `
    precision mediump float;
    varying vec3 vColor;
    void main() {
      gl_FragColor = vec4(vColor, 1.0);
    }
  `;

  function compileShader(type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      console.error(gl.getShaderInfoLog(shader));
      gl.deleteShader(shader);
      return null;
    }
    return shader;
  }

  const vertexShader = compileShader(gl.VERTEX_SHADER, vsSource);
  const fragmentShader = compileShader(gl.FRAGMENT_SHADER, fsSource);

  const program = gl.createProgram();
  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);
  gl.linkProgram(program);
  gl.useProgram(program);

  // position (x, y) + color (r, g, b)
  const vertices = new Float32Array([
     0.0,  0.5,   1.0, 0.0, 0.0,  // r
    -0.5, -0.5,   0.0, 1.0, 0.0,  // g
     0.5, -0.5,   0.0, 0.0, 1.0,  // b
  ]);

  const vbo = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
  gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

  const aPosition = gl.getAttribLocation(program, 'aPosition');
  gl.vertexAttribPointer(aPosition, 2, gl.FLOAT, false, 5 * 4, 0);
  gl.enableVertexAttribArray(aPosition);

  const aColor = gl.getAttribLocation(program, 'aColor');
  gl.vertexAttribPointer(aColor, 3, gl.FLOAT, false, 5 * 4, 2 * 4);
  gl.enableVertexAttribArray(aColor);

  gl.clearColor(0.0, 0.0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, 3);
}
