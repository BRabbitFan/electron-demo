#include <napi-inl.h>
#include <napi.h>

using namespace std::literals;

auto GetHelloWorld(const Napi::CallbackInfo& info) -> Napi::String {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "Hello World"s);
}

auto Init(Napi::Env env, Napi::Object exports) -> Napi::Object {
  exports.Set(Napi::String::New(env, "get_message"s), Napi::Function::New(env, GetHelloWorld));
  return exports;
}

NODE_API_MODULE(addon, Init)
