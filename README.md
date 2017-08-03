# AeonEngine [![Build status](https://ci.appveyor.com/api/projects/status/github/AeonGames/AeonEngine?branch=master&svg=true)](https://ci.appveyor.com/project/Kwizatz/AeonEngine/branch/master) [![Build Status](https://travis-ci.org/AeonGames/AeonEngine.svg?branch=master)](https://travis-ci.org/AeonGames/AeonEngine) [![Coverity Scan Build Status](https://scan.coverity.com/projects/10765/badge.svg)](https://scan.coverity.com/projects/aeongames-aeonengine)
Aeon Games Flagship Game Engine

Check out the [development blog](http://www.aeongames.com/AeonEngine/) for the engine's latest news.

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

THIS IS A WORK IN PROGRESS.

Building:
=========
Ubuntu 14.04 and up:
--------------------

## Install required Packages
    sudo apt-get install -y sed python python3 python-autopep8 python-pep8 python3-pep8 tar wget cmake autoconf automake libtool curl make g++ unzip zlib1g-dev libpng12-dev vim-common qtbase5-dev astyle
    
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

Visual Studio 2015:
-------------------

Building with Visual Studio is somewhat more involved as all dependencies and tools need to be build first, for this I have created a separate project, the [AeonGames Runtime](https://github.com/AeonGames/runtime), you might want to build and install that first, then point the CMake variable RUNTIME_INSTALL_PREFIX to the path where it was installed when configuring.

In the future, building should be possible using [Microsoft's vcpkg](https://github.com/Microsoft/vcpkg), but as it is right now it is lacking support for binary tools such as astyle, sed, xxd and protoc which are required for building and contributing to this repo.

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
    - Windows is the OS I spend most time on, I tend to use a lot of open source code that was born on Linux, but most of them have native Windows ports.
