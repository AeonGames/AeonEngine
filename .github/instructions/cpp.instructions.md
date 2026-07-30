---
description: "AeonEngine C++ style: naming prefixes, astyle formatting, header guards, license block, and public vs private header rules. Applies to all C/C++/Objective-C++ sources."
applyTo: "**/*.{h,hpp,cpp,mm,c}"
---

# AeonEngine C++ conventions

Everything lives in `namespace AeonGames`.

## Formatting

Enforced by astyle (config: [astylerc](../../astylerc)) and applied automatically by the
CMake-installed `.git/hooks/pre-commit`. Match it when you write code so the hook is a no-op:

- Allman braces (opening brace on its own line), 4 spaces, no tabs, namespace bodies indented.
- Spaces around binary operators and **inside parentheses**: `if ( a == b )`, `Foo ( x, y )`.
- Always brace single-statement conditionals.

The pre-commit hook also strips trailing whitespace, refreshes the copyright year, and runs
`autopep8` on Python and `cmake-format` on CMake files.

## Naming

| Kind | Rule | Example |
| --- | --- | --- |
| Types | PascalCase | `ModelComponent`, `Matrix4x4` |
| Member variables | `m` + PascalCase | `mFieldOfView`, `mCells` |
| Function parameters | `a` + PascalCase | `aFieldOfView`, `aFrustum`, `aDelta` |
| Constants / `constexpr` | `k` + PascalCase | `kMaxLocationCodeDepth`, `kPi` |
| Enums | `enum class`, PascalCase members | `enum class LogLevel { Debug, … }` |

## Headers

- Public API: `.hpp` in [include/aeongames](../../include/aeongames), exported with the `DLL` macro
  from [Platform.hpp](../../include/aeongames/Platform.hpp). Private/implementation headers: `.h`
  next to their `.cpp` under `engine/`.
- Include guards only — `#ifndef AEONGAMES_<FILENAME>_H` / `#define` / `#endif`. Never `#pragma once`.
- Every file starts with the Apache-2.0 block; keep the existing comma-separated year list and let
  the hook append the current year:

```cpp
/*
Copyright (C) <years> Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
```

## Errors, logging, docs

- Throw `std::runtime_error` with a descriptive message for unrecoverable errors; there is no error
  code convention in the public API.
- Log through the stream manipulator in [LogLevel.hpp](../../include/aeongames/LogLevel.hpp):
  `std::cout << LogLevel::Error << message << std::endl;` — no logger class.
- OpenGL code uses `OPENGL_CHECK_ERROR_THROW` / `OPENGL_CHECK_ERROR_NO_THROW` around GL calls.
- Public declarations carry Doxygen `@brief` / `@param` / `@return`; group overrides with
  `/** @name Overrides */ ///@{ … ///@}`. Use `@todo` for known gaps.

## Reminders

- No recursive functions — iterate with an explicit, preferably stack-allocated, stack.
- Prefer existing domain classes (`AABB`, `Plane`, `Transform`, `Quaternion`) over loose primitive
  bundles in signatures.
- Ownership flows through `std::unique_ptr`; returned raw pointers are non-owning and must not be
  deleted.
