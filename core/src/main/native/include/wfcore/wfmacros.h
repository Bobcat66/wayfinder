#pragma once

// Cross-platform GCC macros
#ifdef __GNUC__
#define WF_FORCEINLINE __attribute__((always_inline))
#define WF_NOINLINE __attribute__((noinline))
#endif // __GNUC__

// Cross-platform MSVC macros
#ifdef _MSC_VER
#define WF_FORCEINLINE __forceinline
#define WF_NOINLINE __declspec(noinline)
#endif // _MSC_VER

// Windows specific macros
#ifdef _WIN32

// Macros for cross-compiling from GCC
#ifdef __GNUC__
#define WF_DLLEXPORT __attribute__((dllexport))
#define WF_DLLIMPORT __attribute__((dllimport))
#define WF_GCC_HIDDEN
#endif // __GNUC__

// Macros for windows native compiles
#ifdef _MSC_VER
#define WF_DLLEXPORT __declspec(dllexport)
#define WF_DLLIMPORT __declspec(dllimport)
#define WF_GCC_HIDDEN
#endif // _MSC_VER

#endif // _WIN32

// Linux specific macros
#ifdef __linux__

// Compiler-agnostic macros

// For linux native builds
#ifdef __GNUC__
#define WF_DLLEXPORT
#define WF_DLLIMPORT
#define WF_GCC_HIDDEN __attribute__((visibility ("hidden")))
#endif // __GNUC__

#endif // __linux__