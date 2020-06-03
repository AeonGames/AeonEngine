/*
Copyright (C) 2020 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <node_api.h>
#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/Window.h"
#include "aeongames/Scene.h"
#include "Window.h"
#include "Scene.h"

napi_value GetRendererConstructorNames ( napi_env env, napi_callback_info info )
{
    napi_value renderer_array;
    std::vector<std::string> renderers = AeonGames::GetRendererConstructorNames();
    napi_status status = napi_create_array_with_length ( env, renderers.size(), &renderer_array );
    assert ( status == napi_ok );
    for ( size_t i = 0; i < renderers.size(); ++i )
    {
        napi_value renderer_name;
        status = napi_create_string_utf8 ( env, renderers[i].c_str(), renderers[i].size(), &renderer_name );
        assert ( status == napi_ok );
        status = napi_set_element ( env, renderer_array, i, renderer_name );
        assert ( status == napi_ok );
    }
    return renderer_array;
}

napi_value SetRenderer ( napi_env env, napi_callback_info info )
{
    size_t argc{1};
    napi_value argv{};
    napi_status status = napi_get_cb_info ( env, info, &argc, &argv, nullptr, nullptr );
    assert ( status == napi_ok );
    if ( argc > 0 )
    {
        size_t bufsize{0};
        napi_get_value_string_utf8 ( env, argv, nullptr, 0, &bufsize );
        std::vector<char> renderer_name ( size_t{bufsize + 1} );
        napi_get_value_string_utf8 ( env, argv, renderer_name.data(), renderer_name.size(), &bufsize );
        AeonGames::SetRenderer ( renderer_name.data() );
    }
    return nullptr;
}

napi_value Initialize ( napi_env env, napi_value exports )
{
    std::cout << __func__ << std::endl;
    napi_status status;
    static bool initialized = false;
    if ( !initialized )
    {
        if ( ! ( initialized = AeonGames::InitializeGlobalEnvironment() ) )
        {
            napi_throw_error ( env, "InitializeGlobalEnvironment", "Failed to initialize global environment." );
            return exports;
        }
        napi_add_env_cleanup_hook ( env, [] ( void* )
        {
            AeonGames::FinalizeGlobalEnvironment();
        }, nullptr );
    }
    static std::array<napi_property_descriptor, 2> descriptors
    {
        {
            { "getRendererConstructorNames", 0, GetRendererConstructorNames, 0, 0, 0, napi_default, 0 },
            { "setRenderer", 0, SetRenderer, 0, 0, 0, napi_default, 0 },
        }
    };
    status = napi_define_properties ( env, exports, descriptors.size(), descriptors.data() );
    assert ( status == napi_ok );

    InitializeWindow ( env, exports );
    InitializeScene ( env, exports );

    return exports;
}

NAPI_MODULE ( NODE_GYP_MODULE_NAME, Initialize )
