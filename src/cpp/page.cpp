// src/cpp/page.cpp
#include "page.h"
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>
#include <algorithm>

Napi::FunctionReference Page::constructor;

Napi::Object Page::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  
  Napi::Function func = DefineClass(env, "Page", {
    InstanceMethod("getSize", &Page::GetSize),
    InstanceMethod("getText", &Page::GetText),
    InstanceMethod("renderToImage", &Page::RenderToImage),
    InstanceMethod("getRotation", &Page::GetRotation),
    InstanceMethod("getDuration", &Page::GetDuration),
    InstanceMethod("getTextBoxes", &Page::GetTextBoxes),
    InstanceMethod("exportToJSON", &Page::ExportToJSON)
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

Napi::Value Page::GetTextBoxes(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  try {
    std::vector<poppler::text_box> textBoxes = page_->text_list();
    Napi::Array result = Napi::Array::New(env, textBoxes.size());
    
    for (size_t i = 0; i < textBoxes.size(); ++i) {
      const auto& box = textBoxes[i];
      Napi::Object textBoxObj = Napi::Object::New(env);
      
      // Get text
      poppler::byte_array utf8_bytes = box.text().to_utf8();
      std::string text(utf8_bytes.data(), utf8_bytes.size());
      textBoxObj.Set("text", Napi::String::New(env, text));
      
      // Get bounding box
      Napi::Object bbox = Napi::Object::New(env);
      bbox.Set("x", Napi::Number::New(env, box.bbox().x()));
      bbox.Set("y", Napi::Number::New(env, box.bbox().y()));
      bbox.Set("width", Napi::Number::New(env, box.bbox().width()));
      bbox.Set("height", Napi::Number::New(env, box.bbox().height()));
      textBoxObj.Set("bbox", bbox);
      
      result.Set(i, textBoxObj);
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::Error::New(env, std::string("Error getting text boxes: ") + e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value Page::ExportToJSON(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!page_) {
    Napi::Error::New(env, "Page not initialized").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  try {
    // Get text boxes
    std::vector<poppler::text_box> textBoxes = page_->text_list();
    
    // Group text boxes into lines
    std::vector<std::vector<poppler::text_box>> lines = groupTextBoxesIntoLines(std::move(textBoxes));
    
    // Sort lines by Y coordinate (top to bottom)
    std::sort(lines.begin(), lines.end(), [](const std::vector<poppler::text_box>& a, 
                                            const std::vector<poppler::text_box>& b) {
      if (a.empty() || b.empty()) return false;
      return a[0].bbox().y() > b[0].bbox().y();
    });
    
    // Build JSON structure
    Napi::Object result = Napi::Object::New(env);
    
    // Page info
    Napi::Object pageInfo = Napi::Object::New(env);
    auto rect = page_->page_rect();
    pageInfo.Set("width", Napi::Number::New(env, rect.width()));
    pageInfo.Set("height", Napi::Number::New(env, rect.height()));
    pageInfo.Set("rotation", Napi::Number::New(env, static_cast<int>(page_->orientation())));
    result.Set("page", pageInfo);
    
    // Lines array
    Napi::Array linesArray = Napi::Array::New(env, lines.size());
    
    for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
      auto& line = lines[lineIdx];
      
      // Sort words in line by X coordinate
      std::sort(line.begin(), line.end(), [](const poppler::text_box& a, 
                                            const poppler::text_box& b) {
        return a.bbox().x() < b.bbox().x();
      });
      
      Napi::Object lineObj = Napi::Object::New(env);
      
      // Line text
      std::string lineText;
      for (size_t i = 0; i < line.size(); ++i) {
        if (i > 0) lineText += " ";
        poppler::byte_array utf8_bytes = line[i].text().to_utf8();
        lineText += std::string(utf8_bytes.data(), utf8_bytes.size());
      }
      lineObj.Set("text", Napi::String::New(env, lineText));
      
      // Line bounding box
      if (!line.empty()) {
        double minX = line[0].bbox().x();
        double maxX = line[0].bbox().x() + line[0].bbox().width();
        double minY = line[0].bbox().y();
        double maxY = line[0].bbox().y() + line[0].bbox().height();
        
        for (const auto& box : line) {
          minX = std::min(minX, box.bbox().x());
          maxX = std::max(maxX, box.bbox().x() + box.bbox().width());
          minY = std::min(minY, box.bbox().y());
          maxY = std::max(maxY, box.bbox().y() + box.bbox().height());
        }
        
        Napi::Object lineBbox = Napi::Object::New(env);
        lineBbox.Set("x", Napi::Number::New(env, minX));
        lineBbox.Set("y", Napi::Number::New(env, minY));
        lineBbox.Set("width", Napi::Number::New(env, maxX - minX));
        lineBbox.Set("height", Napi::Number::New(env, maxY - minY));
        lineObj.Set("bbox", lineBbox);
      }
      
      // Words array
      Napi::Array wordsArray = Napi::Array::New(env, line.size());
      for (size_t wordIdx = 0; wordIdx < line.size(); ++wordIdx) {
        const auto& box = line[wordIdx];
        Napi::Object wordObj = Napi::Object::New(env);
        
        poppler::byte_array utf8_bytes = box.text().to_utf8();
        std::string wordText(utf8_bytes.data(), utf8_bytes.size());
        wordObj.Set("text", Napi::String::New(env, wordText));
        
        Napi::Object wordBbox = Napi::Object::New(env);
        wordBbox.Set("x", Napi::Number::New(env, box.bbox().x()));
        wordBbox.Set("y", Napi::Number::New(env, box.bbox().y()));
        wordBbox.Set("width", Napi::Number::New(env, box.bbox().width()));
        wordBbox.Set("height", Napi::Number::New(env, box.bbox().height()));
        wordObj.Set("bbox", wordBbox);
        
        wordsArray.Set(wordIdx, wordObj);
      }
      lineObj.Set("words", wordsArray);
      
      linesArray.Set(lineIdx, lineObj);
    }
    
    result.Set("lines", linesArray);
    
    return result;
  } catch (const std::exception& e) {
    Napi::Error::New(env, std::string("Error exporting to JSON: ") + e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

std::vector<std::vector<poppler::text_box>> Page::groupTextBoxesIntoLines(
    std::vector<poppler::text_box>&& textBoxes) {
  
  std::vector<std::vector<poppler::text_box>> lines;
  const double lineThreshold = 5.0; // Points tolerance for same line
  
  for (auto& box : textBoxes) {
    bool addedToExistingLine = false;
    
    // Try to add to existing line
    for (auto& line : lines) {
      if (!line.empty()) {
        double lineY = line[0].bbox().y();
        double boxY = box.bbox().y();
        
        if (std::abs(lineY - boxY) < lineThreshold) {
          line.push_back(std::move(box));
          addedToExistingLine = true;
          break;
        }
      }
    }
    
    // Create new line if not added to existing one
    if (!addedToExistingLine) {
      std::vector<poppler::text_box> newLine;
      newLine.push_back(std::move(box));
      lines.push_back(std::move(newLine));
    }
  }
  
  return lines;
}