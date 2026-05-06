#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include <SDL3/SDL.h>

namespace brabbit {

class GlRenderer final {
 public:
  explicit GlRenderer();
  ~GlRenderer();

  GlRenderer(const GlRenderer&) = delete;
  GlRenderer& operator=(const GlRenderer&) = delete;


 public:
  auto create(int width, int height) -> bool;
  auto destroy() -> void;
  auto render() -> const uint8_t*;
  auto resize(int width, int height) -> void;

  // Apply mouse drag delta (in pixels) to rotation.
  auto rotate(float dx, float dy) -> void;

  auto getWidth() const -> int { return width_; }
  auto getHeight() const -> int { return height_; }

 private:
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>             window_{ nullptr, SDL_DestroyWindow };
  std::unique_ptr<SDL_GLContextState, decltype(&SDL_GL_DestroyContext)> gl_context_{ nullptr, SDL_GL_DestroyContext };

  unsigned int shader_program_{ 0 };
  unsigned int vao_{ 0 };
  unsigned int vbo_{ 0 };
  unsigned int ebo_{ 0 };

  // Offscreen FBO.
  unsigned int fbo_{ 0 };
  unsigned int color_tex_{ 0 };
  unsigned int depth_rbo_{ 0 };

  // Double-buffered PBO for async readback.
  std::array<unsigned int, 2> pbo_{ 0, 0 };
  int                         pbo_index_{ 0 };
  bool                        pbo_ready_{ false };

  int width_{ 0 };
  int height_{ 0 };

  // Rotation (cumulative angles in radians).
  float rot_x_{ 0.3f };  // slight initial tilt
  float rot_y_{ 0.5f };
};

}  // namespace brabbit
