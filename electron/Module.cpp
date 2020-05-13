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

static napi_ref WindowConstructor{};
static napi_ref SceneConstructor{};

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

napi_value CreateWindowProxy ( napi_env env, napi_callback_info info )
{
    size_t argc{1};
    napi_value this_arg{};
    napi_value argv{};
    napi_value target;
    napi_status status{};

    status = napi_get_new_target ( env, info, &target );
    assert ( status == napi_ok );
    if ( target != nullptr )
    {

        status = napi_get_cb_info ( env, info, &argc, &argv, &this_arg, nullptr );
        assert ( status == napi_ok );
        void* window_id{nullptr};
        if ( argc > 0 )
        {
            bool is_buffer{};
            napi_is_buffer ( env, argv, &is_buffer );
            if ( is_buffer )
            {
                size_t size{sizeof ( void* ) };
                status = napi_get_buffer_info ( env, argv, &window_id, &size );
                window_id = reinterpret_cast<void*> ( *reinterpret_cast<uintptr_t*> ( window_id ) );
                assert ( status == napi_ok );
            }
        }
        if ( window_id == nullptr )
        {
            return nullptr;
        }

        const AeonGames::Renderer* renderer{AeonGames::GetRenderer() };
        if ( !renderer )
        {
            return nullptr;
        }

        status = napi_wrap ( env,
                             this_arg,
                             renderer->CreateWindowProxy ( window_id ).release(),
                             [] ( napi_env env, void* finalize_data, void* finalize_hint )->void
        {
            delete reinterpret_cast<AeonGames::Window*> ( finalize_data );
        },
        nullptr,
        nullptr );
        assert ( status == napi_ok );
        return this_arg;
    }
    napi_value constructor{};
    napi_value instance{};
    status = napi_get_cb_info ( env, info, &argc, &argv, nullptr, nullptr );
    assert ( status == napi_ok );
    status = napi_get_reference_value ( env, WindowConstructor, &constructor );
    assert ( status == napi_ok );
    status = napi_new_instance ( env, constructor, argc, &argv, &instance );
    assert ( status == napi_ok );
    return instance;
}

napi_value CreateScene ( napi_env env, napi_callback_info info )
{
    size_t argc{1};
    napi_value this_arg{};
    napi_value argv{};
    napi_value target;
    napi_status status{};

    status = napi_get_new_target ( env, info, &target );
    assert ( status == napi_ok );
    if ( target != nullptr )
    {

        status = napi_get_cb_info ( env, info, &argc, &argv, &this_arg, nullptr );
        assert ( status == napi_ok );
        std::vector<char> scene_file{};
        if ( argc > 0 )
        {
            size_t bufsize{0};
            napi_get_value_string_utf8 ( env, argv, nullptr, 0, &bufsize );
            scene_file.resize ( size_t{bufsize + 1} );
            napi_get_value_string_utf8 ( env, argv, scene_file.data(), scene_file.size(), &bufsize );
            AeonGames::SetRenderer ( scene_file.data() );
        }

        //@todo Add scene constructor taking a scene filename
        AeonGames::Scene* scene = new AeonGames::Scene{};

        if ( scene_file.size() > 0 )
        {
            scene->Load ( scene_file.data() );
        }

        status = napi_wrap ( env,
                             this_arg,
                             scene,
                             [] ( napi_env env, void* finalize_data, void* finalize_hint )->void
        {
            delete reinterpret_cast<AeonGames::Scene*> ( finalize_data );
        },
        nullptr,
        nullptr );
        assert ( status == napi_ok );
        return this_arg;
    }
    napi_value constructor{};
    napi_value instance{};
    status = napi_get_cb_info ( env, info, &argc, &argv, nullptr, nullptr );
    assert ( status == napi_ok );
    status = napi_get_reference_value ( env, SceneConstructor, &constructor );
    assert ( status == napi_ok );
    status = napi_new_instance ( env, constructor, argc, &argv, &instance );
    assert ( status == napi_ok );
    return instance;
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
    static std::array<napi_property_descriptor, 3> descriptors
    {
        {
            { "getRendererConstructorNames", 0, GetRendererConstructorNames, 0, 0, 0, napi_default, 0 },
            { "setRenderer", 0, SetRenderer, 0, 0, 0, napi_default, 0 },
            { "createWindowProxy", 0, CreateWindowProxy, 0, 0, 0, napi_default, 0 },
        }
    };
    status = napi_define_properties ( env, exports, descriptors.size(), descriptors.data() );
    assert ( status == napi_ok );

    napi_value constructor{};

    status = napi_define_class ( env,
                                 "Window",
                                 NAPI_AUTO_LENGTH,
                                 CreateWindowProxy,
                                 nullptr,
                                 0,
                                 nullptr,
                                 &constructor );
    assert ( status == napi_ok );
    status = napi_set_named_property ( env, exports, "Window", constructor );
    assert ( status == napi_ok );

    status = napi_create_reference ( env, constructor, 1, &WindowConstructor );
    assert ( status == napi_ok );

    constructor = nullptr;
    status = napi_define_class ( env,
                                 "Scene",
                                 NAPI_AUTO_LENGTH,
                                 CreateScene,
                                 nullptr,
                                 0,
                                 nullptr,
                                 &constructor );
    assert ( status == napi_ok );
    status = napi_set_named_property ( env, exports, "Scene", constructor );
    assert ( status == napi_ok );

    status = napi_create_reference ( env, constructor, 1, &SceneConstructor );
    assert ( status == napi_ok );
    return exports;
}

NAPI_MODULE ( NODE_GYP_MODULE_NAME, Initialize )
