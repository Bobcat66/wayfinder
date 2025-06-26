/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


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