#include <format>

#include <napi-inl.h>
#include <napi.h>

#include <SDL3/SDL.h>
#include <brabbit/node_apis.hh>

using namespace std::literals;

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

auto Init(Napi::Env env, Napi::Object exports) -> Napi::Object {
  exports.Set(Napi::String::New(env, "SetNodeApis"s), Napi::Function::New(env, SetNodeApis));
  exports.Set(Napi::String::New(env, "GetNativeApiVersion"s), Napi::Function::New(env, GetNativeApiVersion));
  return exports;
}

NODE_API_MODULE(addon, Init)
