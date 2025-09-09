// src/cpp/page.h
#ifndef PAGE_H
#define PAGE_H

#include <napi.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <memory>

class Page : public Napi::ObjectWrap<Page> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env, std::unique_ptr<poppler::page> page);
  
  Page(const Napi::CallbackInfo& info);
  
  // Instance methods
  Napi::Value GetSize(const Napi::CallbackInfo& info);
  Napi::Value GetText(const Napi::CallbackInfo& info);
  Napi::Value RenderToImage(const Napi::CallbackInfo& info);
  Napi::Value GetRotation(const Napi::CallbackInfo& info);
  Napi::Value GetDuration(const Napi::CallbackInfo& info);
  
  // Getters
  poppler::page* GetPage() const { return page_.get(); }

private:
  std::unique_ptr<poppler::page> page_;
  static Napi::FunctionReference constructor;
};

#endif // PAGE_H