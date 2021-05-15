/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANUTILITIES_H
#define AEONGAMES_VULKANUTILITIES_H
#include "vulkan/vulkan.h"
#ifdef Status
#undef Status
#endif
#include "aeongames/Pipeline.h"
namespace AeonGames
{
    static_assert ( VK_SUCCESS == 0, "VK_SUCCESS is NOT zero!" );
    const char* GetVulkanResultString ( VkResult aResult );

    struct Vertex
    {
        float position[3];
        float normal[3];
        float tangent[3];
        float bitangent[3];
        float uv[2];
        uint8_t weight_indices[4];
        uint8_t weight_influences[4];
        float color[3];
    };

    static_assert (
        sizeof ( Vertex ) ==
        ( sizeof ( float ) * 17 + sizeof ( uint8_t ) * 8 ),
        "Vertex Structure Contains Padding." );

    VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback (
        VkFlags aFlags,
        VkDebugReportObjectTypeEXT aObjType,
        uint64_t aSrcObject,
        size_t aLocation,
        int32_t aCode,
        const char *aLayerPrefix,
        const char *aMsg,
        void *aUserData );
}
#endif
