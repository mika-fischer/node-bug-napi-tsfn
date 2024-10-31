{
  "targets": [
    {
      "target_name": "addon",
      "sources": ["src/addon.cpp"],
      "cflags_cc": ["--std=c++20"],
      'cflags!': [ '-fno-exceptions', '-fno-rtti' ],
      'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
    }
  ]
}