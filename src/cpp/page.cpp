// src/cpp/page.cpp
#include "page.h"
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>

Napi::FunctionReference Page::constructor;

Napi::Object Page::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  
  Napi::Function func = DefineClass(env, "Page", {
    InstanceMethod("getSize", &Page::GetSize),
    InstanceMethod("getText", &Page::GetText),
    InstanceMethod("renderToImage", &Page::RenderToImage),
    InstanceMethod("getRotation", &Page::GetRotation),
    InstanceMethod("getDuration", &Page::GetDuration)
  });
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  
  exports.Set("Page", func);
  return exports;
}

Napi::Object Page::NewInstance(Napi::Env env, std::unique_ptr<poppler::page> page) {
  Napi::Object instance = constructor.New({});
  Page* pageWrapper = Napi::ObjectWrap<Page>::Unwrap(instance);
  pageWrapper->page_ = std::move(page);
  return instance;
}

Page::Page(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Page>(info) {
  // Private constructor
}

Napi::Value Page::GetSize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  poppler::rectf size = page_->page_rect();
  
  Napi::Object result = Napi::Object::New(env);
  result.Set("width", Napi::Number::New(env, size.width()));
  result.Set("height", Napi::Number::New(env, size.height()));
  result.Set("x", Napi::Number::New(env, size.x()));
  result.Set("y", Napi::Number::New(env, size.y()));
  
  return result;
}

Napi::Value Page::GetText(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  poppler::ustring text = page_->text();
  poppler::byte_array utf8_bytes = text.to_utf8();
  std::string utf8_text(utf8_bytes.data(), utf8_bytes.size());
  return Napi::String::New(env, utf8_text);
}

Napi::Value Page::RenderToImage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Default rendering options
  double dpi = 72.0;
  int rotation = 0;
  
  // Parse options if provided
  if (info.Length() > 0 && info[0].IsObject()) {
    Napi::Object options = info[0].As<Napi::Object>();
    
    if (options.Has("dpi")) {
      dpi = options.Get("dpi").As<Napi::Number>().DoubleValue();
    }
    
    if (options.Has("rotation")) {
      rotation = options.Get("rotation").As<Napi::Number>().Int32Value();
    }
  }
  
  try {
    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    
    poppler::image image = renderer.render_page(page_.get(), dpi, dpi, 
                                               -1, -1, -1, -1, 
                                               static_cast<poppler::rotation_enum>(rotation));
    
    if (!image.is_valid()) {
      Napi::Error::New(env, "Failed to render page").ThrowAsJavaScriptException();
      return env.Null();
    }
    
    // Convert image to buffer
    const char* data = image.const_data();
    int width = image.width();
    int height = image.height();
    int bytes_per_row = image.bytes_per_row();
    
    // Create buffer with image data
    size_t dataSize = height * bytes_per_row;
    Napi::Buffer<char> buffer = Napi::Buffer<char>::Copy(env, data, dataSize);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("data", buffer);
    result.Set("width", Napi::Number::New(env, width));
    result.Set("height", Napi::Number::New(env, height));
    result.Set("bytesPerRow", Napi::Number::New(env, bytes_per_row));
    result.Set("format", Napi::String::New(env, "ARGB32")); // Poppler uses ARGB32
    
    return result;
  } catch (const std::exception& e) {
    Napi::Error::New(env, std::string("Error rendering page: ") + e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value Page::GetRotation(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  return Napi::Number::New(env, static_cast<int>(page_->orientation()));
}

Napi::Value Page::GetDuration(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  return Napi::Number::New(env, page_->duration());
}