// src/cpp/poppler_binding.cpp
#include <napi.h>
#include "document.h"
#include "page.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // Initialize Document class
  Document::Init(env, exports);
  
  // Initialize Page class
  Page::Init(env, exports);
  
  // Add utility functions
  exports.Set("version", Napi::String::New(env, "0.0.0"));
  
  return exports;
}

NODE_API_MODULE(poppler_binding, Init)