// src/cpp/document.cpp
#include "document.h"
#include "page.h"
#include <poppler/cpp/poppler-document.h>

Napi::FunctionReference Document::constructor;

Napi::Object Document::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  
  Napi::Function func = DefineClass(env, "Document", {
    StaticMethod("loadFromFile", &Document::LoadFromFile),
    StaticMethod("loadFromBuffer", &Document::LoadFromBuffer),
    InstanceMethod("getPageCount", &Document::GetPageCount),
    InstanceMethod("getPage", &Document::GetPage),
    InstanceMethod("getMetadata", &Document::GetMetadata),
    InstanceMethod("isLocked", &Document::IsLocked),
    InstanceMethod("unlock", &Document::Unlock)
  });
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  
  exports.Set("Document", func);
  return exports;
}

Document::Document(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Document>(info) {
  // Private constructor - use static factory methods
}

Napi::Value Document::LoadFromFile(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected string filepath").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  std::string filepath = info[0].As<Napi::String>().Utf8Value();
  std::string password = "";
  
  if (info.Length() > 1 && info[1].IsString()) {
    password = info[1].As<Napi::String>().Utf8Value();
  }
  
  try {
    poppler::document* raw_doc;
    if (password.empty()) {
      raw_doc = poppler::document::load_from_file(filepath);
    } else {
      raw_doc = poppler::document::load_from_file(filepath, password);
    }
    
    std::unique_ptr<poppler::document> doc(raw_doc);
    
    if (!doc) {
      Napi::Error::New(env, "Failed to load PDF document").ThrowAsJavaScriptException();
      return env.Null();
    }
    
    // Create new Document instance
    Napi::Object instance = constructor.New({});
    Document* docWrapper = Napi::ObjectWrap<Document>::Unwrap(instance);
    docWrapper->doc_ = std::shared_ptr<poppler::document>(doc.release());
    
    return instance;
  } catch (const std::exception& e) {
    Napi::Error::New(env, std::string("Error loading document: ") + e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value Document::LoadFromBuffer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::TypeError::New(env, "Expected Buffer").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
  std::string password = "";
  
  if (info.Length() > 1 && info[1].IsString()) {
    password = info[1].As<Napi::String>().Utf8Value();
  }
  
  try {
    poppler::document* raw_doc;
    if (password.empty()) {
      raw_doc = poppler::document::load_from_raw_data(buffer.Data(), buffer.Length());
    } else {
      raw_doc = poppler::document::load_from_raw_data(buffer.Data(), buffer.Length(), password);
    }
    
    std::unique_ptr<poppler::document> doc(raw_doc);
    
    if (!doc) {
      Napi::Error::New(env, "Failed to load PDF document from buffer").ThrowAsJavaScriptException();
      return env.Null();
    }
    
    // Create new Document instance
    Napi::Object instance = constructor.New({});
    Document* docWrapper = Napi::ObjectWrap<Document>::Unwrap(instance);
    docWrapper->doc_ = std::shared_ptr<poppler::document>(doc.release());
    
    return instance;
  } catch (const std::exception& e) {
    Napi::Error::New(env, std::string("Error loading document: ") + e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value Document::GetPageCount(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!doc_) {
    Napi::Error::New(env, "Document not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  return Napi::Number::New(env, doc_->pages());
}

Napi::Value Document::GetPage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!doc_) {
    Napi::Error::New(env, "Document not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Expected page number").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  int pageIndex = info[0].As<Napi::Number>().Int32Value();
  
  if (pageIndex < 0 || pageIndex >= doc_->pages()) {
    Napi::RangeError::New(env, "Page index out of range").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  std::unique_ptr<poppler::page> page(doc_->create_page(pageIndex));
  if (!page) {
    Napi::Error::New(env, "Failed to create page").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  return Page::NewInstance(env, std::move(page));
}

Napi::Value Document::GetMetadata(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!doc_) {
    Napi::Error::New(env, "Document not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  Napi::Object metadata = Napi::Object::New(env);
  
  // Get document info
  std::vector<std::string> keys = doc_->info_keys();
  for (const std::string& key : keys) {
    poppler::ustring value = doc_->info_key(key);
    poppler::byte_array utf8_bytes = value.to_utf8();
    std::string utf8_value(utf8_bytes.data(), utf8_bytes.size());
    metadata.Set(key, Napi::String::New(env, utf8_value));
  }
  
  return metadata;
}

Napi::Value Document::IsLocked(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!doc_) {
    Napi::Error::New(env, "Document not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  return Napi::Boolean::New(env, doc_->is_locked());
}

Napi::Value Document::Unlock(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!doc_) {
    Napi::Error::New(env, "Document not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected password string").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  std::string password = info[0].As<Napi::String>().Utf8Value();
  bool success = doc_->unlock(password, password); // owner_password, user_password
  
  return Napi::Boolean::New(env, success);
}