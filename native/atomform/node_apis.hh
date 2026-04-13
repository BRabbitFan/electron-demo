#pragma once

#include <map>

#include <napi-inl.h>
#include <napi.h>

using ApiMap = std::map<std::string, Napi::FunctionReference, std::less<>>;

class NodeApiProxy final {
 public:
  explicit NodeApiProxy(std::string_view name, ApiMap& apis) : name_{ name }, apis_{ apis }, iter_{ apis_.end() } {}

  auto visit() -> Napi::FunctionReference& {
    if (iter_ == apis_.end()) {
      iter_ = apis_.find(name_);
      if (iter_ == apis_.end()) {
        iter_ = apis_.emplace(name_, Napi::FunctionReference{}).first;
      }
    }

    return iter_->second;
  }

  operator bool() {
    return !visit().IsEmpty();
  }

  auto operator->() -> Napi::FunctionReference* {
    return &visit();
  }

 private:
  std::string_view name_;
  ApiMap&          apis_;
  ApiMap::iterator iter_;
};

inline class NodeApis final {
 public:
  auto emplace(Napi::FunctionReference&& function) -> void {
    auto name = function.Value().Get("name").As<Napi::String>().Utf8Value();
    apis_.emplace(std::move(name), std::move(function));
  }

#define NODE_API(name) NodeApiProxy name{ #name, apis_ }
  // ---------- real APIs declaration (begin) ----------

  NODE_API(GetNodeApiVersion);

  // ---------- real APIs declaration (end) ----------
#undef NODE_API

 private:
  ApiMap apis_{};
} NODE_APIS;
