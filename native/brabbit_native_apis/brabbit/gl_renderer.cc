#include <string_view>

#include <glad/glad.h>
//
#include <SDL3/SDL.h>

#include <brabbit/gl_renderer.hh>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std::literals;

namespace brabbit {

  namespace {

    constexpr auto VERTEX_SHADER_SOURCE = R"glsl(
      #version 410 core
      layout(location = 0) in vec3 aPosition;
      layout(location = 1) in vec3 aColor;
      uniform mat4 uMVP;
      out vec3 vColor;
      void main() {
        gl_Position = uMVP * vec4(aPosition, 1.0);
        vColor = aColor;
      }
    )glsl"sv;

    constexpr auto FRAGMENT_SHADER_SOURCE = R"glsl(
      #version 410 core
      in vec3 vColor;
      out vec4 fragColor;
      void main() {
        fragColor = vec4(vColor, 1.0);
      }
    )glsl"sv;

    // Cube: 24 vertices (4 per face, unique colors per face), 36 indices.
    // Each vertex: position(3) + color(3).
    // clang-format off
    constexpr auto CUBE_VERTICES = std::array{
      // Front face (red)
      -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
      // Back face (green)
       0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
      // Top face (blue)
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
      // Bottom face (yellow)
      -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
      // Right face (magenta)
       0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
      // Left face (cyan)
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
    };

    constexpr auto CUBE_INDICES = std::array{
       0u,  1u,  2u,   2u,  3u,  0u,  // front
       4u,  5u,  6u,   6u,  7u,  4u,  // back
       8u,  9u, 10u,  10u, 11u,  8u,  // top
      12u, 13u, 14u,  14u, 15u, 12u,  // bottom
      16u, 17u, 18u,  18u, 19u, 16u,  // right
      20u, 21u, 22u,  22u, 23u, 20u,  // left
    };

  }  // namespace

  GlRenderer::GlRenderer() = default;

  GlRenderer::~GlRenderer() {
    destroy();
  }

  auto GlRenderer::create(int width, int height) -> bool {
    if (window_) {
      return false;
    }

    width_ = width;
    height_ = height;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    auto properties = SDL_CreateProperties();
    SDL_SetStringProperty(properties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "gl_offscreen");
    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1);
    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 1);
    SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);

    window_.reset(SDL_CreateWindowWithProperties(properties));
    SDL_DestroyProperties(properties);
    if (!window_) {
      return false;
    }

    gl_context_.reset(SDL_GL_CreateContext(window_.get()));
    if (!gl_context_) {
      destroy();
      return false;
    }

    SDL_GL_MakeCurrent(window_.get(), gl_context_.get());
    SDL_GL_SetSwapInterval(0);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
      destroy();
      return false;
    }

    // --- FBO with depth ---
    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &color_tex_);
    glGenRenderbuffers(1, &depth_rbo_);

    glBindTexture(GL_TEXTURE_2D, color_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      destroy();
      return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- PBOs ---
    const auto buffer_size = GLsizeiptr{ width_ * height_ * 4 };
    glGenBuffers(2, pbo_.data());
    for (const auto pbo : pbo_) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
      glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, nullptr, GL_STREAM_READ);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    pbo_index_ = 0;
    pbo_ready_ = false;

    // --- Shaders ---
    const auto compile_shader = [](GLenum type, const char* source) -> GLuint {
      auto shader = glCreateShader(type);
      glShaderSource(shader, 1, &source, nullptr);
      glCompileShader(shader);

      auto success = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glDeleteShader(shader);
        return 0;
      }

      return shader;
    };

    auto vs = compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE.data());
    auto fs = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE.data());
    if (!vs || !fs) {
      destroy();
      return false;
    }

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vs);
    glAttachShader(shader_program_, fs);
    glLinkProgram(shader_program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    auto linked = 0;
    glGetProgramiv(shader_program_, GL_LINK_STATUS, &linked);
    if (!linked) {
      destroy();
      return false;
    }

    // --- Cube geometry ---
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES), CUBE_INDICES.data(), GL_STATIC_DRAW);

    // aPosition
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // aColor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    return true;
  }

  auto GlRenderer::destroy() -> void {
    if (window_ && gl_context_) {
      SDL_GL_MakeCurrent(window_.get(), gl_context_.get());

      if (fbo_) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
      }

      if (color_tex_) {
        glDeleteTextures(1, &color_tex_);
        color_tex_ = 0;
      }

      if (depth_rbo_) {
        glDeleteRenderbuffers(1, &depth_rbo_);
        depth_rbo_ = 0;
      }

      if (vao_) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
      }

      if (vbo_) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
      }

      if (ebo_) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
      }

      if (shader_program_) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
      }
    }
  }

  auto GlRenderer::render() -> const uint8_t* {
    if (!window_ || !gl_context_) {
      return nullptr;
    }

    SDL_GL_MakeCurrent(window_.get(), gl_context_.get());

    // Unmap previously mapped PBO.
    if (pbo_ready_) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_[pbo_index_]);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    // --- Build MVP ---
    auto aspect = static_cast<float>(width_) / static_cast<float>(height_);
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = glm::lookAt(glm::vec3{ 0, 0, 3 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 1, 0 });
    auto model = glm::mat4(1.0f);
    model = glm::rotate(model, rot_x_, glm::vec3{ 1, 0, 0 });
    model = glm::rotate(model, rot_y_, glm::vec3{ 0, 1, 0 });
    auto mvp = projection * view * model;

    // --- Draw scene to FBO ---
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program_);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // --- Async PBO readback ---
    auto curr_index = pbo_index_;
    auto next_index = 1 - pbo_index_;
    pbo_index_ = next_index;

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_.at(curr_index));
    glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const uint8_t* pixels = nullptr;
    if (pbo_ready_) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_.at(next_index));
      pixels = reinterpret_cast<const uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    pbo_ready_ = true;
    return pixels;
  }

  auto GlRenderer::resize(int width, int height) -> void {
    if (!window_ || !gl_context_) {
      return;
    }

    if (width == width_ && height == height_) {
      return;
    }

    SDL_GL_MakeCurrent(window_.get(), gl_context_.get());

    width_ = width;
    height_ = height;

    for (const auto pbo : pbo_) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, color_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    const auto buffer_size = GLsizeiptr{ width_ * height_ * 4 };
    for (const auto pbo : pbo_) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
      glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, nullptr, GL_STREAM_READ);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    pbo_ready_ = false;
  }

  auto GlRenderer::rotate(float dx, float dy) -> void {
    const auto sensitivity = float{ 0.005f };
    rot_y_ += dx * sensitivity;
    rot_x_ += dy * sensitivity;
  }

}  // namespace brabbit
