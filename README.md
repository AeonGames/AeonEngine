# AeonEngine

[![MS Visual C++ Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml) [![MSYS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml) [![Linux Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-linux.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-linux.yml) [![macOS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml) [![CodeFactor](https://www.codefactor.io/repository/github/aeongames/AeonEngine/badge)](https://www.codefactor.io/repository/github/aeongames/AeonEngine) [![Patreon](https://img.shields.io/badge/patreon-donate-orange.svg)](https://www.patreon.com/user?u=3798744) [![Ko-Fi](https://img.shields.io/badge/ko--fi-donate-orange.svg)](https://ko-fi.com/aeongames)

## Aeon Games Flagship Game Engine

[![Aeon Games](https://www.aeongames.com/AeonBlack.svg)](https://aeongames.com)

![Sponza Runtime Render on Vulkan](https://www.aeongames.com/screenshots/SponzaVulkanWindows.png)

AeonEngine is a cross-platform, plugin-based 3D game engine written in C++20. It supports multiple rendering backends, uses Protocol Buffers for asset serialization, and integrates with Blender for content creation workflows.

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

> **⚠️ THIS IS A WORK IN PROGRESS.**

---

## ✨ Features

### Rendering

- **Vulkan** — Primary renderer with SPIR-V shader compilation via glslang. On macOS, Vulkan is provided through MoltenVK.
- **OpenGL 4.5** — Secondary renderer using core profile. Disabled on macOS (Apple does not support OpenGL 4.5).
- **Metal 3** — Native renderer for Apple Silicon and macOS 13+, using argument buffers, bindless resources, indirect command buffers, and Metal shader interface metadata generated from the shared GLSL sources.
- **Shared render protocol** — `Renderer::RenderScene` owns one backend-neutral frame sequence while each plugin implements its API-specific resource binding and command recording: descriptor sets on Vulkan, reflected flat bindings on OpenGL, and argument buffers on Metal.
- **Compute pipelines** — A unified Pipeline asset can carry both graphics and multiple ordered compute stages. Every backend supports storage buffers and transient per-frame memory; skeletal meshes are skinned on the GPU in a compute pre-pass that writes posed vertices into a buffer the mesh is drawn from.
- **Clustered Forward+ lighting** — A compute-driven light culling pipeline bins lights into view-space clusters (`cluster_build`), culls them per cluster (`light_cull`, including a cone-vs-cluster test for spot lights), and packs a global light-index list via an atomic allocator. An optional depth pre-pass marks active clusters so only visible clusters are shaded. Per-frame lights are uploaded as an SSBO (cap 4096). A cluster light-count heatmap debug view is available.
- **GPU visibility and submission** — CPU octree/frustum culling builds the render queue, then Hi-Z compute culling rejects static instances before indirect submission. Vulkan/OpenGL use shared geometry pools; Metal compacts same-mesh batches into indirect command ranges.
- **Shadows** — Directional, spot, and point-light shadow maps, including cached point shadows and six-face multiview rendering where the backend supports it.
- **HDR and reflections** — Linear `RGBA16F` scene rendering, environment skybox and GGX-prefiltered image-based lighting, screen-space reflections, deferred specular composition, ACES tone mapping, and explicit sRGB output transfer.
- **Bindless materials** — Renderer-owned texture/sampler tables and GPU material records allow indirect batches to shade multiple materials without rebinding each draw.

### Engine Subsystems

| Subsystem | Description |
| ----------- | ------------- |
| **Scene Graph** | Hierarchical node-based scene management with component system |
| **Math** | Vector2/3/4, Quaternion, Matrix3x3/4x4, Transform, AABB, Frustum, Plane |
| **Lighting** | Point, spot, and directional lights with radius attenuation and cone falloff; per-pixel Blinn-Phong shading; clustered Forward+ light culling. Per-frame lights collected on the Scene and uploaded to the GPU. |
| **Materials** | Phong material model (`Kd`, `Ks`, `Shininess`) with texture samplers; `Ks`/`Shininess` carried in the Material UBO |
| **Skeletal Animation** | Bone hierarchies, keyframe animation, skeleton/animation resources; GPU compute skinning runs as a pre-pass that poses vertices into a buffer the mesh is then drawn from |
| **Sound** | Audio via PortAudio with Ogg Vorbis decoding |
| **Resource Cache** | Centralized resource loading with caching and factory pattern |
| **GUI Overlay** | Optional in-engine GUI via [AeonGUI](https://github.com/AeonGames/AeonGUI) (Cairo backend) |

### Components

- **Camera / FreeCamera / OrbitalCamera / OverTheShoulderCamera** — Fixed, free-fly, orbit, and third-person camera behavior
- **ModelComponent** — Model rendering component (mesh + material + pipeline)
- **PointLight / SpotLight / DirectionalLight** — Scene illumination with radius attenuation and spot-cone falloff
- **CharacterController** — Movement/turn controller wired to the input system

### Asset Pipeline

All game assets are serialized using [Protocol Buffers](https://protobuf.dev/), including meshes, materials, pipelines, skeletons, animations, scenes, and models. Shader pipelines under `game/shaders/` and cooked Blender outputs under `game/<asset>/` are generated from sources under `assets/`; edit the source shader or `.blend`, not the generated file.

### Tools

- **aeontool** — Command-line utility for asset conversion (binary ↔ text), packaging, base64 encoding, pipeline compilation/variants, game indexes, and character-library transcoding. See [tools/aeontool/README.md](tools/aeontool/README.md).
- **WorldEditor** — Qt6-based GUI editor for scene and node hierarchy editing, component management, property inspection, and renderer selection.
- **Blender Addons** — Export meshes, materials, skeletons, animations, models, collisions, images, and complete scenes directly from Blender. The scene exporter preserves instancing and creates light, camera, marker, and environment data. See [tools/blender/README.md](tools/blender/README.md).
- **metal-shader-tool** — macOS shader cooker that turns shared GLSL/SPIR-V into validated MSL 3.0 and records Metal argument-buffer interface metadata.

### Platforms

| Platform | Toolchains | Notes |
| ---------- | ----------- | ------- |
| **Windows** | MSVC (Visual Studio 2022+), MSYS2 (MinGW64, Clang64, UCRT64) | Full support (Vulkan + OpenGL) |
| **Linux** | GCC, Clang | Full support (Vulkan + OpenGL) |
| **macOS** | Apple Clang, Apple Silicon | Native Metal 3 and Vulkan via MoltenVK; OpenGL 4.5 is disabled |

### Documentation

- [Renderer architecture and backend contract](engine/renderers/README.md) - shared frame protocol, plugin anatomy, reflection/bindings, native Metal architecture, and backend validation
- [Vulkan render pipeline](engine/renderers/vulkan/RENDER_PIPELINE.md) - frame-by-frame clustered Forward+, shadows, GPU culling, HDR, synchronization, and descriptor design
- [Shader authoring](.github/instructions/shaders.instructions.md) - cross-backend GLSL branches, renderer variants, binding rules, and pipeline regeneration
- [Blender asset pipeline](tools/blender/README.md) - headless cooks, interactive exporters, scene instancing, and addon packaging
- [AeonTool command reference](tools/aeontool/README.md) - conversion, packages, pipelines, indexes, and character-library tools

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
    gtest:p \
    sqlite:p
```

Repeat for each subplatform you want to target.

#### 📥 Clone and Build with MSYS2

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -G "MSYS Makefiles" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### 🧪 Run MSYS2 Tests

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
    libpango1.0-dev \
    libxml2-dev \
    libjpeg-dev \
    libsqlite3-dev \
    sqlite3 \
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
    vulkan-validationlayers \
    flex \
    bison
```

#### 📥 Clone and Build on Linux

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

#### 🧪 Run Linux Tests

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

#### 📥 Clone and Build with Visual Studio

Open a VS Developer Command Prompt or Developer PowerShell:

```cmd
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -B build
cmake --build build --config Release
```

Or open the generated `.sln` file in Visual Studio and build from the IDE.

### 🍎 macOS

> **Note:** The native Metal renderer requires Apple Silicon, macOS 13+, argument-buffer tier 2,
> and Apple's separately downloaded Metal compiler. OpenGL 4.5 is not supported on macOS;
> Vulkan remains available through MoltenVK. The example below enables AeonGUI, which raises the
> deployment target to macOS 13.3+.

#### 🍺 Install Dependencies with Homebrew

Make sure [Homebrew](https://brew.sh/) is installed, then:

```bash
brew update
brew install \
    flex \
    bison \
    gnu-sed \
    cmake \
    ninja \
    protobuf \
    zlib \
    libpng \
    glslang \
    spirv-cross \
    portaudio \
    libogg \
    libvorbis \
    cairo \
    pango \
    libxml2 \
    libjpeg-turbo \
    googletest \
    qt6 \
    pkg-config \
    molten-vk \
    spirv-headers \
    spirv-tools \
    vulkan-extensionlayer \
    vulkan-headers \
    vulkan-loader \
    vulkan-profiles \
    vulkan-tools \
    vulkan-utility-libraries \
    vulkan-validationlayers \
    vulkan-volk
```

Install Apple's Metal compiler once through Xcode:

```bash
xcodebuild -downloadComponent MetalToolchain
```

#### 📥 Clone and Build on macOS

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
export PATH="$(brew --prefix bison)/bin:$PATH"
cmake -G Ninja -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_AEONGUI=ON \
    -DBUILD_METAL_RENDERER=ON \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3
cmake --build build --parallel "$(sysctl -n hw.ncpu)"
```

#### 🧪 Run macOS Tests

```bash
cd build
ctest --output-on-failure
```

---

## ▶️ Running

Run from the repository root because [game/config](game/config) and package paths are relative to it:

```bash
# Windows/MSYS2
PATH="$PWD/build/bin:$PATH" ./build/bin/game.exe -r Vulkan -s scenes/main.txt

# Linux
PATH="$PWD/build/bin:$PATH" ./build/bin/game -r OpenGL -s scenes/main.txt

# Apple Silicon/macOS
PATH="$PWD/build/bin:$PATH" ./build/bin/game -r Metal -s scenes/main.txt
```

Renderer names are exactly `OpenGL`, `Vulkan`, and `Metal` where built. Use `-f` for fullscreen,
`-p <file.png>` to capture a frame, and `-n <frame>` to choose the capture frame (default: 30).

## 🔄 Generated Shaders and Blender Assets

Sources under [assets/shadercode](assets/shadercode) generate `game/shaders/*.txt` and `.pln` files:

```bash
cmake --build build --target shader-pipelines
```

When Metal is enabled, this target also runs `metal-shader-tool`; it requires `glslangValidator`,
`spirv-cross`, the macOS SDK, and Apple's downloaded Metal compiler.

Blender assets are opt-in because cooking can be expensive. CMake exposes `aerin`, `backdrop`,
`polesign`, and `sponza` targets when Blender is available:

```bash
cmake --build build --target aerin
```

The `sponza` target exports a full scene and one model per unique mesh datablock. See the
[Blender asset pipeline](tools/blender/README.md) for setup and output details.

---

## ⚙️ CMake Options

| Option | Default | Description |
| -------- | --------- | ------------- |
| `BUILD_VULKAN_RENDERER` | `ON` | Build the Vulkan renderer plugin |
| `BUILD_OPENGL_RENDERER` | `ON` | Build the OpenGL 4.5 renderer plugin (forced `OFF` on macOS) |
| `BUILD_METAL_RENDERER` | Apple Silicon macOS: `ON`; otherwise: `OFF` | Build the native Metal 3 renderer plugin (requires macOS 13+, Apple Silicon, and argument-buffer tier 2) |
| `BUILD_STANDALONE_APPLICATION` | `ON` | Build the standalone application/viewer |
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

```text
AeonEngine/
├── application/     # Standalone game launcher/viewer
├── assets/          # Bundled demo assets (Aerin model, Sponza scene)
├── cmake/           # CMake modules, toolchain files, and templates
├── engine/          # Core engine library
│   ├── components/  #   Camera, ModelComponent, PointLight
│   ├── core/        #   Scene, Node, Renderer, Pipeline, Material, Mesh, etc.
│   ├── databases/   #   Database plugins (SQLite)
│   ├── gui/         #   AeonGUI integration (optional)
│   ├── images/      #   PNG and Radiance HDR image plugins
│   ├── input/       #   Desktop input plugin
│   ├── math/        #   Vector, Matrix, Quaternion, Transform, AABB, Frustum
│   ├── renderers/   #   Vulkan, OpenGL, and Metal renderer plugins
│   └── sound/       #   PortAudio + Ogg Vorbis audio
├── game/            # Game data (scenes, shaders, materials, meshes, models)
├── include/         # Public engine headers (aeongames/)
├── proto/           # Protocol Buffer definitions for all asset types
├── tests/           # GTest unit tests
├── tools/
│   ├── aeontool/    #   CLI asset conversion and packaging tool
│   ├── blender/     #   Blender exporter addons
│   ├── metalshader/ #   GLSL/SPIR-V reflection and Metal shader cooker
│   └── worldeditor/ #   Qt6 scene editor GUI
└── vcpkg-port/      # Custom vcpkg port overlays
```

---

## 📜 License

Licensed under the [Apache License, Version 2.0](LICENSE.md).
