# AeonEngine

[![MS Visual C++ Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml) [![MSYS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml) [![Linux Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-linux.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-linux.yml) [![macOS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml) [![CodeFactor](https://www.codefactor.io/repository/github/aeongames/AeonEngine/badge)](https://www.codefactor.io/repository/github/aeongames/AeonEngine) [![Patreon](https://img.shields.io/badge/patreon-donate-orange.svg)](https://www.patreon.com/user?u=3798744) [![Ko-Fi](https://img.shields.io/badge/ko--fi-donate-orange.svg)](https://ko-fi.com/aeongames)

## Aeon Games Flagship Game Engine

[![Aeon Games](https://www.aeongames.com/AeonBlack.svg)](https://aeongames.com)

AeonEngine is a cross-platform, plugin-based 3D game engine written in C++. It supports multiple rendering backends, uses Protocol Buffers for asset serialization, and integrates with Blender for content creation workflows.

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

> **⚠️ THIS IS A WORK IN PROGRESS.**

---

## ✨ Features

### Rendering

- **Vulkan** — Primary renderer with SPIR-V shader compilation via glslang. On macOS, Vulkan is provided through MoltenVK.
- **OpenGL 4.5** — Secondary renderer using core profile. Disabled on macOS (Apple does not support OpenGL 4.5).
- **Minimum common denominator** design — All uniforms use Uniform Buffer Objects (UBOs) so the same shaders work identically across Vulkan, OpenGL, and potential future backends (DirectX, Metal).

### Engine Subsystems

| Subsystem | Description |
|-----------|-------------|
| **Scene Graph** | Hierarchical node-based scene management with component system |
| **Math** | Vector2/3/4, Quaternion, Matrix3x3/4x4, Transform, AABB, Frustum, Plane |
| **Materials** | Material property system with texture samplers |
| **Skeletal Animation** | Bone hierarchies, keyframe animation, skeleton/animation resources |
| **Sound** | Audio via PortAudio with Ogg Vorbis decoding |
| **Resource Cache** | Centralized resource loading with caching and factory pattern |
| **GUI Overlay** | Optional in-engine GUI via [AeonGUI](https://github.com/AeonGames/AeonGUI) (Cairo backend) |

### Components

- **Camera** — First-person/third-person camera component
- **ModelComponent** — Model rendering component (mesh + material + pipeline)
- **PointLight** — Point light for scene illumination

### Asset Pipeline

All game assets are serialized using [Protocol Buffers](https://protobuf.dev/), including meshes, materials, pipelines, skeletons, animations, scenes, and models.

### Tools

- **aeontool** — Command-line utility for asset conversion (binary ↔ text), packaging, base64 encoding, and pipeline compilation.
- **WorldEditor** — Qt6-based GUI editor for scene and node hierarchy editing, component management, property inspection, and renderer selection.
- **Blender Addons** — Export meshes, skeletons, animations, models, collisions, images, and scenes directly from Blender to AeonEngine formats.

### Platforms

| Platform | Toolchains | Notes |
|----------|-----------|-------|
| **Windows** | MSVC (Visual Studio 2022+), MSYS2 (MinGW64, Clang64, UCRT64) | Full support (Vulkan + OpenGL) |
| **Linux** | GCC, Clang | Full support (Vulkan + OpenGL) |
| **macOS** | Apple Clang (via Homebrew) | Vulkan only (via MoltenVK), no standalone application |

---

## 📦 Git LFS

This repository uses [Git Large File Storage (LFS)](https://git-lfs.com/) to manage large binary asset files (meshes, base64-encoded resources, etc.). You **must** install and initialize Git LFS before cloning, otherwise these files will be checked out as small pointer files instead of actual data.

### Quick Setup

1. **Install Git LFS** (one-time per machine):

   - **Windows (MSYS2):** `pacboy -S git-lfs:p`
   - **Arch Linux:** `pacman -S git-lfs`
   - **Ubuntu/Linux:** `sudo apt-get install git-lfs`
   - **macOS:** `brew install git-lfs`
   - **Windows (Git for Windows):** Git LFS is bundled — no extra install needed.

2. **Initialize Git LFS** (one-time per machine):

   ```bash
   git lfs install
   ```

3. **Clone the repository** as usual — LFS files are downloaded automatically:

   ```bash
   git clone https://github.com/AeonGames/AeonEngine.git
   ```

If you already cloned without LFS, run `git lfs pull` inside the repository to download the LFS objects.

The tracked patterns are defined in [.gitattributes](.gitattributes) and currently include `*.msh` and `*.b64` files.

---

## 🚀 Building

The AeonEngine uses CMake and supports building on multiple platforms. Choose the method that best fits your environment.

### 🪟 Windows with MSYS2 MinGW

[Visual Studio Code](https://code.visualstudio.com/) is not required to build, but is highly recommended for development. The project includes VS Code configuration templates for tasks, launch, and settings.

#### 📦 Install MSYS2

Go to [MSYS2](https://www.msys2.org/) and install MSYS2. Development targets 64-bit, so choose that if you are unsure.

#### 📋 Install Required Packages

Open an MSYS2 terminal and update all installed packages:

```bash
pacman -Syuu --noconfirm
```

You may need to close the terminal and run the command again until no more updates are reported.

Install general system tools:

```bash
pacman -S --needed --noconfirm git pactoys make
```

The `pactoys` package provides `pacboy`, which installs packages for specific toolchains. Pick a subplatform (mingw64, clang64, or ucrt64), open the corresponding terminal, and install the required packages:

```bash
pacboy -S --needed --noconfirm \
    toolchain:p \
    cmake:p \
    make:p \
    tools-git:p \
    vulkan:p \
    vulkan-devel:p \
    qt6:p \
    protobuf:p \
    zlib:p \
    libpng:p \
    glslang:p \
    portaudio:p \
    libogg:p \
    libvorbis:p \
    cairo:p \
    gtest:p
```

Repeat for each subplatform you want to target.

#### 📥 Clone and Build

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -G "MSYS Makefiles" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### 🧪 Run Tests

```bash
cd build
ctest --output-on-failure
```

#### Edit with Visual Studio Code

Open the repository root folder in VS Code (File → Open Folder). Go to View → Terminal to get an integrated bash terminal where you can run build commands directly. Running executables from the debug environment uses GDB, supporting breakpoints and the Debug Console.

### 🐧 Ubuntu/Linux

#### 📦 Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    software-properties-common \
    gcc \
    g++ \
    llvm \
    clang \
    sed \
    python3 \
    tar \
    wget \
    cmake \
    autoconf \
    automake \
    libtool \
    curl \
    make \
    unzip \
    zlib1g-dev \
    libpng-dev \
    vim-common \
    git \
    portaudio19-dev \
    libogg-dev \
    libvorbis-dev \
    googletest \
    libglu1-mesa-dev \
    freeglut3-dev \
    mesa-common-dev \
    libcairo2-dev \
    libprotobuf-dev \
    protobuf-compiler \
    mesa-vulkan-drivers \
    libvulkan1 \
    libvulkan-dev \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-l10n-tools \
    libxkbcommon-dev \
    glslang-dev \
    glslang-tools \
    libglx-mesa0 \
    vulkan-validationlayers
```

#### 📥 Clone and Build

With GCC:

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

With Clang:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/clang-toolchain.cmake
cmake --build build
```

#### 🧪 Run Tests

```bash
cd build
ctest --output-on-failure
```

### 🪟 Windows with Visual Studio

Dependency management uses [Microsoft's vcpkg](https://github.com/Microsoft/vcpkg). Requires Visual Studio 2022 or later.

#### Prerequisites

1. **vcpkg** — Install via the Visual Studio Installer. The project includes a `vcpkg.json` manifest that automatically downloads and builds required packages—you just need to point CMake at the `vcpkg.cmake` toolchain file.
2. **Vulkan SDK** — Download and install from [LunarG](https://vulkan.lunarg.com/sdk/home).
3. **Git for Windows** — Download and install from [git-scm.com](https://git-scm.com/downloads/win).

#### 📥 Clone and Build

Open a VS Developer Command Prompt or Developer PowerShell:

```cmd
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -B build
cmake --build build --config Release
```

Or open the generated `.sln` file in Visual Studio and build from the IDE.

### 🍎 macOS

> **Note:** OpenGL 4.5 is not supported on macOS. Only the Vulkan renderer (via MoltenVK) is available. The standalone application is also disabled on macOS.

#### 🍺 Install Dependencies with Homebrew

Make sure [Homebrew](https://brew.sh/) is installed, then:

```bash
brew update
brew install \
    cmake \
    make \
    protobuf \
    zlib \
    libpng \
    glslang \
    portaudio \
    libogg \
    libvorbis \
    cairo \
    googletest \
    qt6 \
    pkg-config \
    molten-vk
```

#### 📥 Clone and Build

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### 🧪 Run Tests

```bash
cd build
ctest --output-on-failure
```

---

## ⚙️ CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_VULKAN_RENDERER` | `ON` | Build the Vulkan renderer plugin |
| `BUILD_OPENGL_RENDERER` | `ON` | Build the OpenGL 4.5 renderer plugin (forced `OFF` on macOS) |
| `BUILD_STANDALONE_APPLICATION` | `ON` | Build the standalone application/viewer (forced `OFF` on macOS) |
| `USE_AEONGUI` | `OFF` | Enable AeonGUI library for in-engine GUI overlays |
| `USE_CLANG_TIDY` | `OFF` | Run clang-tidy static analysis during build (requires clang-tidy) |
| `PROXY` | (empty) | Proxy server URL for network downloads during build |

Example — build with only the Vulkan renderer:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_OPENGL_RENDERER=OFF
cmake --build build
```

---

## 🔧 Contributing

The CMake configuration installs a git pre-commit hook that formats code using **astyle** (C++), **autopep8** (Python/Blender scripts), and **cmake-format** (CMake files). Install the formatters before creating commits:

```bash
python3 -m pip install autopep8 cmake-format
```

---

## 📁 Project Structure

```
AeonEngine/
├── application/     # Standalone game launcher/viewer
├── assets/          # Bundled demo assets (Aerin model, Sponza scene)
├── cmake/           # CMake modules, toolchain files, and templates
├── engine/          # Core engine library
│   ├── components/  #   Camera, ModelComponent, PointLight
│   ├── core/        #   Scene, Node, Renderer, Pipeline, Material, Mesh, etc.
│   ├── gui/         #   AeonGUI integration (optional)
│   ├── images/      #   Image loaders (PNG)
│   ├── math/        #   Vector, Matrix, Quaternion, Transform, AABB, Frustum
│   ├── renderers/   #   Vulkan and OpenGL renderer plugins
│   └── sound/       #   PortAudio + Ogg Vorbis audio
├── game/            # Game data (scenes, shaders, materials, meshes, models)
├── include/         # Public engine headers (aeongames/)
├── proto/           # Protocol Buffer definitions for all asset types
├── tests/           # GTest unit tests
├── tools/
│   ├── aeontool/    #   CLI asset conversion and packaging tool
│   ├── blender/     #   Blender exporter addons
│   └── worldeditor/ #   Qt6 scene editor GUI
└── vcpkg-port/      # Custom vcpkg port overlays
```

---

## 📜 License

Licensed under the [Apache License, Version 2.0](LICENSE.md).
