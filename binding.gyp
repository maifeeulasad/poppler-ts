{
  "targets": [
    {
      "target_name": "poppler_binding",
      "sources": [
        "src/cpp/poppler_binding.cpp",
        "src/cpp/document.cpp",
        "src/cpp/page.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "/usr/include/poppler/cpp",
        "/usr/local/include/poppler/cpp"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "libraries": [
        "-lpoppler-cpp"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        ["OS=='mac'", {
          "include_dirs": [
            "/opt/homebrew/include/poppler/cpp",
            "/usr/local/include/poppler/cpp"
          ],
          "libraries": [
            "-L/opt/homebrew/lib",
            "-L/usr/local/lib"
          ]
        }],
        ["OS=='win'", {
          "include_dirs": [
            "C:/vcpkg/installed/x64-windows/include/poppler/cpp"
          ],
          "libraries": [
            "-LC:/vcpkg/installed/x64-windows/lib",
            "poppler-cpp.lib"
          ]
        }]
      ]
    }
  ]
}