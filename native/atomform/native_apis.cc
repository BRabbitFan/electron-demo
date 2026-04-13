#include <format>
#include <format>

#include <napi-inl.h>
#include <napi.h>

#include <atomform/node_apis.hh>

using namespace std::literals;

auto SetNodeApi(const Napi::CallbackInfo& info) -> Napi::Value {
  auto env = info.Env();
  if (info.Length() < 1 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Expected a function as argument"s).ThrowAsJavaScriptException();
    return env.Null();
  }

  NODE_APIS.emplace(Napi::Persistent(info[0].As<Napi::Function>()));
  return env.Null();
}

auto GetNativeApiVersion(const Napi::CallbackInfo& info) -> Napi::String {
  Napi::Env env = info.Env();

  auto native_version = "v1.0.0"s;

  if (NODE_APIS.GetNodeApiVersion) {
    if (auto node_version = NODE_APIS.GetNodeApiVersion->Call({}); node_version.IsString()) {
      native_version += std::format("(node:{})"sv, node_version.As<Napi::String>().Utf8Value());
    }
  }

  return Napi::String::New(env, native_version);
}

auto Init(Napi::Env env, Napi::Object exports) -> Napi::Object {

  auto object = Napi::Object::New(env);
  object.Set(Napi::String::New(env, "SetNodeApi"s), Napi::Function::New(env, SetNodeApi));
  exports.Set(Napi::String::New(env, "af"s), object);

  object.DefineProperties({
    Napi::PropertyDescriptor::Function(env, object, "SetNodeApi"s, SetNodeApi),
  });

#define NATIVE_API(name) exports.Set(Napi::String::New(env, #name), Napi::Function::New(env, name))
  // ---------- real APIs declaration (begin) ----------

  NATIVE_API(SetNodeApi);
  NATIVE_API(GetNativeApiVersion);

  // ---------- real APIs declaration (end) ----------
#undef NATIVE_API

  return exports;
}

NODE_API_MODULE(addon, Init)
