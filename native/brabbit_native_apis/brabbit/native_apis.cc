#include <format>

#include <napi.h>

#include <brabbit/gl_renderer.hh>
#include <brabbit/node_apis.hh>

using namespace std::literals;

static brabbit::GlRenderer g_renderer;

auto SetNodeApis(const Napi::CallbackInfo& info) -> Napi::Value {
  for (std::size_t index = 0; index < info.Length(); ++index) {
    if (auto api = info[index]; api.IsFunction()) {
      NODE_APIS.emplace(Napi::Persistent(api.As<Napi::Function>()));
    }
  }

  return info.Env().Undefined();
}

auto GetNativeApiVersion(const Napi::CallbackInfo& info) -> Napi::String {
  auto native_version = "v1.0.0"s;

  if (NODE_APIS.GetNodeApiVersion) {
    if (auto node_version = NODE_APIS.GetNodeApiVersion->Call({}); node_version.IsString()) {
      native_version += std::format("(node:{})"sv, node_version.As<Napi::String>().Utf8Value());
    }
  }

  return Napi::String::New(info.Env(), native_version);
}

// CreateGlRenderer(width, height) -> boolean
auto CreateGlRenderer(const Napi::CallbackInfo& info) -> Napi::Value {
  auto env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    return Napi::Boolean::New(env, false);
  }
  int width = info[0].As<Napi::Number>().Int32Value();
  int height = info[1].As<Napi::Number>().Int32Value();
  return Napi::Boolean::New(env, g_renderer.create(width, height));
}

// DestroyGlRenderer() -> void
auto DestroyGlRenderer(const Napi::CallbackInfo& info) -> void {
  g_renderer.destroy();
}

// RenderGl() -> Buffer|null
// Returns RGBA pixel buffer of the previous frame, or null if not ready.
auto RenderGl(const Napi::CallbackInfo& info) -> Napi::Value {
  auto env = info.Env();
  const uint8_t* pixels = g_renderer.render();
  if (!pixels) {
    return env.Null();
  }
  std::size_t size = (std::size_t)g_renderer.getWidth() * g_renderer.getHeight() * 4;
  // Copy pixels into a JS Buffer (the PBO pointer is only valid until next Render).
  return Napi::Buffer<uint8_t>::Copy(env, pixels, size);
}

// ResizeGlRenderer(width, height) -> void
auto ResizeGlRenderer(const Napi::CallbackInfo& info) -> void {
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) return;
  int width = info[0].As<Napi::Number>().Int32Value();
  int height = info[1].As<Napi::Number>().Int32Value();
  g_renderer.resize(width, height);
}

// GetGlRendererSize() -> { width, height }
auto GetGlRendererSize(const Napi::CallbackInfo& info) -> Napi::Value {
  auto env = info.Env();
  auto obj = Napi::Object::New(env);
  obj.Set("width", g_renderer.getWidth());
  obj.Set("height", g_renderer.getHeight());
  return obj;
}

// RotateGlRenderer(dx, dy) -> void
auto RotateGlRenderer(const Napi::CallbackInfo& info) -> void {
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) return;
  float dx = info[0].As<Napi::Number>().FloatValue();
  float dy = info[1].As<Napi::Number>().FloatValue();
  g_renderer.rotate(dx, dy);
}

auto Init(Napi::Env env, Napi::Object exports) -> Napi::Object {
  exports.Set(Napi::String::New(env, "SetNodeApis"s), Napi::Function::New(env, SetNodeApis));
  exports.Set(Napi::String::New(env, "GetNativeApiVersion"s), Napi::Function::New(env, GetNativeApiVersion));
  exports.Set(Napi::String::New(env, "CreateGlRenderer"s), Napi::Function::New(env, CreateGlRenderer));
  exports.Set(Napi::String::New(env, "DestroyGlRenderer"s), Napi::Function::New(env, DestroyGlRenderer));
  exports.Set(Napi::String::New(env, "RenderGl"s), Napi::Function::New(env, RenderGl));
  exports.Set(Napi::String::New(env, "ResizeGlRenderer"s), Napi::Function::New(env, ResizeGlRenderer));
  exports.Set(Napi::String::New(env, "GetGlRendererSize"s), Napi::Function::New(env, GetGlRendererSize));
  exports.Set(Napi::String::New(env, "RotateGlRenderer"s), Napi::Function::New(env, RotateGlRenderer));
  return exports;
}

NODE_API_MODULE(addon, Init)
