#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_GLContextState;

namespace brabbit {

class GlRenderer final {
  SDL_Window* window_ = nullptr;
  SDL_GLContextState* gl_context_ = nullptr;

  unsigned shader_program_ = 0;
  unsigned vao_ = 0;
  unsigned vbo_ = 0;
  unsigned ebo_ = 0;

  // Offscreen FBO.
  unsigned fbo_ = 0;
  unsigned color_tex_ = 0;
  unsigned depth_rbo_ = 0;

  // Double-buffered PBO for async readback.
  unsigned pbo_[2] = {};
  int pbo_index_ = 0;
  bool pbo_ready_ = false;

  int width_ = 0;
  int height_ = 0;

  // Rotation (cumulative angles in radians).
  float rot_x_ = 0.3f;  // slight initial tilt
  float rot_y_ = 0.5f;

 public:
  GlRenderer() = default;
  ~GlRenderer();

  GlRenderer(const GlRenderer&) = delete;
  GlRenderer& operator=(const GlRenderer&) = delete;

  bool Create(int width, int height);
  void Destroy();
  const uint8_t* Render();
  void Resize(int width, int height);

  // Apply mouse drag delta (in pixels) to rotation.
  void Rotate(float dx, float dy);

  int width() const { return width_; }
  int height() const { return height_; }
};

}  // namespace brabbit
