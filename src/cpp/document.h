// src/cpp/document.h
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <napi.h>
#include <poppler/cpp/poppler-document.h>
#include <memory>

class Document : public Napi::ObjectWrap<Document> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Document(const Napi::CallbackInfo& info);
  
  // Static methods
  static Napi::Value LoadFromFile(const Napi::CallbackInfo& info);
  static Napi::Value LoadFromBuffer(const Napi::CallbackInfo& info);
  
  // Instance methods
  Napi::Value GetPageCount(const Napi::CallbackInfo& info);
  Napi::Value GetPage(const Napi::CallbackInfo& info);
  Napi::Value GetMetadata(const Napi::CallbackInfo& info);
  Napi::Value IsLocked(const Napi::CallbackInfo& info);
  Napi::Value Unlock(const Napi::CallbackInfo& info);
  
  // Getters
  std::shared_ptr<poppler::document> GetDocument() const { return doc_; }

private:
  std::shared_ptr<poppler::document> doc_;
  static Napi::FunctionReference constructor;
};

#endif // DOCUMENT_H