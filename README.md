# AeonEngine [![MS Visual C++ Build status](https://ci.appveyor.com/api/projects/status/mps3te4is4xuwnby?svg=true)](https://ci.appveyor.com/project/Kwizatz/aeonengine) [![MinGW64 Build status](https://ci.appveyor.com/api/projects/status/2r8j33fp5eej623u?svg=true)](https://ci.appveyor.com/project/Kwizatz/aeonengine-0t94t) [![Build Status](https://travis-ci.org/AeonGames/AeonEngine.svg?branch=master)](https://travis-ci.org/AeonGames/AeonEngine) [![Coverity Scan Build Status](https://scan.coverity.com/projects/10765/badge.svg)](https://scan.coverity.com/projects/aeongames-aeonengine) [![Patreon](https://img.shields.io/badge/patreon-donate-orange.svg)](https://www.patreon.com/user?u=3798744)

Aeon Games Flagship Game Engine

Check out the [development blog](http://www.aeongames.com/AeonEngine/) for the engine's latest news.

[![Follow us on Facebook](https://raw.githubusercontent.com/AeonGames/AeonEngine/master/docs/static/img/FB-FindUsonFacebook-online-100.png)](https://www.facebook.com/RealAeonGames/)

[![Become a Patron!](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/user?u=3798744)

![Aerin just chilling](https://raw.githubusercontent.com/AeonGames/AeonEngine/master/docs/static/screenshots/aerin_idle.png)

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

THIS IS A WORK IN PROGRESS.

Building:
=========
Ubuntu 14.04 and up:
--------------------

## Install required Packages
    sudo apt-get install -y sed python python3 python-autopep8 python-pep8 python3-pep8 tar wget cmake autoconf automake libtool curl make g++ unzip zlib1g-dev libpng12-dev vim-common qtbase5-dev astyle

## Install GCC 8.x (required for c++17)
    sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get upgrade --allow-unauthenticated
    sudo apt-get install -y build-essential software-properties-common
    sudo apt-get install -y gcc-snapshot
    sudo apt-get install -y gcc-8 g++-8
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8
    
## Build and install libprotobuf-dev from source
    wget https://github.com/google/protobuf/archive/v3.1.0.tar.gz
    tar -xzvf v3.1.0.tar.gz
    cd protobuf-3.1.0
    ./autogen.sh
    ./configure
    make
    sudo make install
    sudo ldconfig
    cd ..

## Build and install glslang from source
    git clone https://github.com/KhronosGroup/glslang.git
    cd glslang
    git checkout 8f674e821e1e5f628474b21d7fe21af2e86b5fb4
    mkdir build
    cd build
    cmake -G "Unix Makefiles" ..
    make
    sudo make install
    sudo ldconfig
    cd ../..
    
## *UBUNTU 14.x*: Locally install libvulkan-dev from self extracting executable into home directory
    cd ~/
    wget https://vulkan.lunarg.com/sdk/download/1.0.30.0/linux/vulkansdk-linux-x86_64-1.0.30.0.run
    chmod a+x vulkansdk-linux-x86_64-1.0.30.0.run
    ./vulkansdk-linux-x86_64-1.0.30.0.run
    echo "VULKAN_SDK=$PWD/VulkanSDK/1.0.30.0/x86_64" >> ~/.bashrc
    echo "export VULKAN_SDK" >> ~/.bashrc
    echo "export PATH=$PWD/VulkanSDK/1.0.30.0/bin:$PATH" >> ~/.bashrc   
    echo "export LD_LIBRARY_PATH=$PWD/VulkanSDK/1.0.30.0/lib:$LD_LIBRARY_PATH" >> ~/.bashrc
    echo "export VK_LAYER_PATH=$PWD/VulkanSDK/1.0.30.0/etc/explicit_layer.d" >> ~/.bashrc
    source ~/.bashrc
    sudo ldconfig
    
## *UBUNTU 16.x*: Use apt to install Vulkan SDK
    sudo apt-get install -y libvulkan-dev

## Generate Makefiles with CMake
    cmake -G "Unix Makefiles" .

## Build
    make

Visual Studio 2017 (version 15.7 or later is required due to c++17's filesystem dependency) and up:
-------------------

Building with Visual Studio is somewhat more involved as all dependencies and tools need to be build first. The official way of doing this is to use [Microsoft's vcpkg](https://github.com/Microsoft/vcpkg), however if you are proficient at building software you can build each dependency individually or you could just find official Windows distributions and install them. You can also pick and chose on what to build and what to install from a previously build distribution, in fact it is recommended to install the Qt5 sdk rather than build it if you want to save about 4+ hours of your life.

## Install VCPKG
    See [Microsoft's vcpkg](https://github.com/Microsoft/vcpkg) README.md for instructions, you can clone the repo anywhere, but I suggest c:\vcpkg to keep it global.
    
## Install Engine dependencies
    In a command prompt window move to the vcpkg root and run the following command:
    .\vcpkg install protobuf zlib libpng glslang spirv-tools
    IF and only IF you have time to space and want to debug Qt5 issues, add qt5 to the list:
    .\vcpkg install protobuf zlib libpng glslang spirv-tools qt5

## Install the Vulkan SDK
    Download and install the Vulkan SDK from [the LunarG website](https://vulkan.lunarg.com/sdk/home).
    The engine supports OpenGL rendering and use of one API or the other is optional, while you could eventually chose not to build either of the renderers, as it is right now, both must be build.
    This is just an oversight rather than a strict policy, and support for disabling modules will be written in the future.

## Install the Qt5 SDK (Only if you did not build Qt5 with VCPKG)
    Download and install the [Qt5 SDK](https://www1.qt.io/download-open-source), you may install it anywhere you want, but in general it is a good idea to avoid paths with spaces in it.

## Generate solution and project files with CMake
    If you're using the GUI, make sure that you add the CMAKE_TOOLCHAIN_FILE variable to point to the vcpkg.cmake file. And if you did not build the Qt SDK,
    set the variable Qt5_DIR to <Qt5 SDK root>/lib/cmake/Qt5 before pressing the configure and generate buttons.
    If you're generating them from the CLI, add the paths to the cmake command:
    cmake -DCMAKE_TOOLCHAIN_FILE:filepath=<VCPKG ROOT>/scripts/buildsystems/vcpkg.cmake -DQt5_DIR:path=<Qt SDK Root>/lib/cmake/Qt5 <PATH TO ENGINE SOURCE ROOT>
    or
    cmake -DCMAKE_TOOLCHAIN_FILE:filepath=<VCPKG ROOT>/scripts/buildsystems/vcpkg.cmake <PATH TO ENGINE SOURCE ROOT>

In No Way Complete TODO List:
=============================

* Find out how to use google::protobuf::TextFormat::Parser so raw mesh bytes can be printed and parsed in a more human readable way.
* Implement a visual programming language gui for shader program authoring (like blueprints).

Unasked Questions Nevertheless Answered (UQNA)
----------------------------------------------

## Why do you use protobuf for your data files?
    Because I've always felt human readability is not worth the price you pay in performace.
## Why do you keep PB plain text files around then?
    They're easier to modify. The idea is that you convert them to binary once you're ready to ship.
## You could do that with &lt;insert favorite human readable format&gt; which is nicer, so why don't you?
    PB's text files are a build in feature, anything else would require a tool to either convert to it,
    directly to binary protocol buffers or use a proprietary format.
    That takes time and Google already solved the problem.
    Do feel free to write your own convertion tool though.
## Why are there so many "Linux Build Fix" commits?
    I develop on Windows first and then make sure things work properly on Linux. Things sometimes break.
    While I do not hold a particular preference towards Windows, there are some things that keep me working on it:
    - Visual Studio's Debugger is the best there is. I do know how to use gdb, but I am not as proficient with it as with VS, and I am yet to find a graphical frontend I like for it.
    - Most PC gamers are on Windows, that's probably not going to change anytime soon.
    - Windows is the OS I spend most time on, I tend to use a lot of open source code that was born on Linux, but most of it has native Windows ports.
