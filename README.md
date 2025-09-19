# AeonEngine

[![MS Visual C++ Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-windows.yml) [![MSYS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-msys2.yml) [![Linux Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-ubuntu.yml) [![macOS Build status](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml/badge.svg)](https://github.com/AeonGames/AeonEngine/actions/workflows/build-macos.yml) [![CodeFactor](https://www.codefactor.io/repository/github/aeongames/AeonEngine/badge)](https://www.codefactor.io/repository/github/aeongames/AeonEngine) [![Patreon](https://img.shields.io/badge/patreon-donate-orange.svg)](https://www.patreon.com/user?u=3798744) [![Ko-Fi](https://img.shields.io/badge/ko--fi-donate-orange.svg)](https://ko-fi.com/aeongames)

## Aeon Games Flagship Game Engine

![Aerin just chilling](https://raw.githubusercontent.com/AeonGames/AeonEngine/master/docs/static/screenshots/aerin_idle.png)

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

THIS IS A WORK IN PROGRESS.

## 🚀 Building

The AeonEngine supports building on multiple platforms with different toolchains. Choose the method that best fits your development environment.

### 🪟 Windows with Visual Studio Code and MSYS2 MinGW

You do not need to install [Visual Studio Code](https://code.visualstudio.com/) just to build the project,
but it is highly recommended that you do so if you intend on changing the code, or if you want to develop a game using MSYS2/MinGW.

#### 📦 Install MSYS2

Go to [MSYS2](https://www.msys2.org/) and install MSYS2, while the 32 bit version of the MinGW compiler should work, development is focused on 64 bit, so get that if you don't know what to chose.

#### 📋 Install required Packages

Bring up an MSYS2 bash terminal for MinGW and update all of your installed packages:

```bash
pacman -Syuu --no-confirm
```

Follow the instructions, you may have to forcefully shut down the terminal and run the same command at least one time.
After pacman reports no more updates, its time to install all our engine dependencies.

First install general required system tools:

```bash
pacman -S --needed --noconfirm git pactoys make
```

The pactoys package installs pacboy which allows instalation of the required packages for the different toolchains as required, so pick a subplatform, either mingw64, clang64 or ucrt64, run the corresponding terminal and proceed to install the required packages:
```bash
pacboy -S --needed --noconfirm \
    toolchain:p \
    cmake:p \
    make:p \
    tools-git:p \
    vulkan:p \
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

This has to be done for each required subplatform.

#### Install autopep8 and cmake-format (optional, only if you want to create a pull request, or make changes to your own fork)

The CMake script installs a git pre-commit hook to format code using astyle, autopep8 (for the Blender scripts) and cmake-format,
so you will need these if you want to create any commits.

```bash
python3 -m pip install autopep8 cmake-format
```

#### Clone the Repository

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
```

#### Build with CMake

```bash
cmake -G "MSYS Makefiles" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Run Tests

```bash
cd build
ctest --output-on-failure
```

#### Edit with Visual Studio Code

You can now use the "Open Folder" option in VS Code to open the topmost repo folder and then go to View->Terminal,
where you'll get prompted to allow bash to run, accept and now you can issue your make commands directly from inside VS Code.

You should be able to run the various executables directly from the terminal or from the debug environment,
if you run them from the debug environment they will be run through GDB, so you can set breakpoints or issue commands from the Debug Console.

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

#### 📥 Clone the Repository

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
```

#### 🔨 Build with CMake

For GCC:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For Clang:
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

Building with Visual Studio uses [Microsoft's vcpkg](https://github.com/Microsoft/vcpkg) for dependency management. The engine requires Visual Studio 2022 or later.

#### 📦 VCPKG

The vcpkg executable can now be installed directly from the Visual Studio Updater, the easiest is to install that, the project is already set to automatically download and build the required packages, you may just need to find where the scripts\buildsystems\vcpkg.cmake script is to feed it to cmake.

#### 🌋 Install the Vulkan SDK

Download and install the Vulkan SDK from [the LunarG website](https://vulkan.lunarg.com/sdk/home).

#### 🌋 Install Git for Windows

Download and install Git for Windows from [git-scm.com](https://git-scm.com/downloads/win).

#### 📥 Clone the Repository

Open a VS Developer terminal, it can be a simple CMD terminal or Powershell.

```cmd
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
```

#### 🔨 Generate Solution Files with CMake

```cmd
cmake -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -B build
```

#### 🚧 Build

```cmd
cmake --build build
```

Or open the generated solution file in Visual Studio and build from the IDE.

### 🍎 macOS

#### 🍺 Install Dependencies with Homebrew

First, make sure you have [Homebrew](https://brew.sh/) installed, then install the required dependencies:

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

#### 📥 Clone the Repository

```bash
git clone https://github.com/AeonGames/AeonEngine.git
cd AeonEngine
```

#### 🔨 Build with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### 🧪 Run Tests

```bash
cd build
ctest --output-on-failure
```

## Miscelaneus

### Enable/Disable Renderers

The BUILD_&lt;API NAME&gt;_RENDERER cmake variable can be used to disable or enable renderers, by default all renderers available on the platform are build, unless one is temporarily disabled due to it being broken or under construction.

In general the following command can be used:

```bash
cmake -DBUILD_VULKAN_RENDERER:boolean=&lt;ON/OFF&gt; -DBUILD_OPENGL_RENDERER:boolean=&lt;ON/OFF&gt; ..
```
