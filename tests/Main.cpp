/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <iostream>
#include "gtest/gtest.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/stubs/common.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.hpp"
#ifdef _WIN32
// Prefer the discrete GPU on hybrid-graphics laptops (NVIDIA Optimus / AMD
// PowerXpress); the drivers only honor these symbols in the executable's own
// export table, not in a linked DLL.
extern "C" {
    __declspec ( dllexport ) unsigned long NvOptimusEnablement = 1;
    __declspec ( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
int main ( int argc, char **argv )
{
    testing::InitGoogleTest ( &argc, argv );
    AeonGames::InitializeGlobalEnvironment();
    int result = RUN_ALL_TESTS();
    AeonGames::FinalizeGlobalEnvironment();
    return result;
}
