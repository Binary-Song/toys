#pragma once

#ifdef LLAMA_WIN
#define LLAMA_EXPORT_SYMBOL __declspec(dllexport)
#define LLAMA_IMPORT_SYMBOL __declspec(dllimport)
#elif
#define LLAMA_EXPORT_SYMBOL __attribute__((visibility("default")))
#define LLAMA_IMPORT_SYMBOL __attribute__((visibility("default")))
#endif