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
#include <array>
#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/Window.h"
#include "aeongames/Scene.h"
#include "Window.h"

static napi_ref WindowConstructor{};

napi_value Window ( napi_env env, napi_callback_info info )
{
    size_t argc{5};
    std::array<napi_value, 5> argv{};
    napi_value this_arg{};
    napi_value target;
    napi_status status{};

    const AeonGames::Renderer* renderer{AeonGames::GetRenderer() };
    if ( !renderer )
    {
        return nullptr;
    }

    status = napi_get_new_target ( env, info, &target );
    assert ( status == napi_ok );
    if ( target != nullptr )
    {
        status = napi_get_cb_info ( env, info, &argc, argv.data(), &this_arg, nullptr );
        assert ( status == napi_ok );
        void* window_id{nullptr};
        if ( argc == 1 )
        {
            bool is_buffer{};
            napi_is_buffer ( env, argv[0], &is_buffer );
            if ( is_buffer )
            {
                size_t size{sizeof ( void* ) };
                status = napi_get_buffer_info ( env, argv[0], &window_id, &size );
                window_id = reinterpret_cast<void*> ( *reinterpret_cast<uintptr_t*> ( window_id ) );
                assert ( status == napi_ok );
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
        }
        else if ( argc > 2 )
        {
            int32_t x;
            int32_t y;
            uint32_t width;
            uint32_t height;
            bool fullScreen{false};

            status = napi_get_value_int32 ( env, argv[0], &x );
            assert ( status == napi_ok );
            status = napi_get_value_int32 ( env, argv[1], &y );
            assert ( status == napi_ok );
            status = napi_get_value_uint32 ( env, argv[2], &width );
            assert ( status == napi_ok );
            status = napi_get_value_uint32 ( env, argv[3], &height );
            assert ( status == napi_ok );
            if ( argc > 4 )
            {
                status = napi_get_value_bool ( env, argv[4], &fullScreen );
                assert ( status == napi_ok );
            }

            status = napi_wrap ( env,
                                 this_arg,
                                 renderer->CreateWindowInstance ( x, y, width, height, fullScreen ).release(),
                                 [] ( napi_env env, void* finalize_data, void* finalize_hint )->void
            {
                delete reinterpret_cast<AeonGames::Window*> ( finalize_data );
            },
            nullptr,
            nullptr );
            assert ( status == napi_ok );
            return this_arg;
        }
        return nullptr;
    }
    napi_value constructor{};
    napi_value instance{};
    status = napi_get_cb_info ( env, info, &argc, argv.data(), nullptr, nullptr );
    assert ( status == napi_ok );
    status = napi_get_reference_value ( env, WindowConstructor, &constructor );
    assert ( status == napi_ok );
    status = napi_new_instance ( env, constructor, argc, argv.data(), &instance );
    assert ( status == napi_ok );
    return instance;
}

napi_value SetScene ( napi_env env, napi_callback_info info )
{
    size_t argc{1};
    napi_value argv{};
    napi_value this_arg{};
    napi_status status = napi_get_cb_info ( env, info, &argc, &argv, &this_arg, nullptr );
    assert ( status == napi_ok );
    if ( argc > 0 )
    {
        AeonGames::Window* window{};
        AeonGames::Scene* scene{};
        napi_unwrap ( env, this_arg, reinterpret_cast<void**> ( &window ) );
        assert ( status == napi_ok );
        napi_unwrap ( env, argv, reinterpret_cast<void**> ( &scene ) );
        assert ( status == napi_ok );
        window->SetScene ( scene );
        window->StartRenderTimer();
    }
    return nullptr;
}

napi_value Show ( napi_env env, napi_callback_info info )
{
    size_t argc{1};
    napi_value argv{};
    napi_value this_arg{};
    napi_status status = napi_get_cb_info ( env, info, &argc, &argv, &this_arg, nullptr );
    assert ( status == napi_ok );
    if ( argc > 0 )
    {
        bool show;
        AeonGames::Window* window{};
        napi_get_value_bool ( env, argv, &show );
        assert ( status == napi_ok );
        napi_unwrap ( env, this_arg, reinterpret_cast<void**> ( &window ) );
        assert ( status == napi_ok );
        window->Show ( show );
    }
    return nullptr;
}

void InitializeWindow ( napi_env env, napi_value exports )
{
    napi_value constructor{};

    static napi_property_descriptor descriptors[] =
    {
        { "show", 0, Show, 0, 0, 0, napi_default, 0 },
        { "setScene", 0, SetScene, 0, 0, 0, napi_default, 0 },
    };

    napi_status status = napi_define_class ( env,
                         "Window",
                         NAPI_AUTO_LENGTH,
                         Window,
                         nullptr,
                         sizeof ( descriptors ) / sizeof ( descriptors[0] ),
                         descriptors,
                         &constructor );
    assert ( status == napi_ok );
    status = napi_set_named_property ( env, exports, "Window", constructor );
    assert ( status == napi_ok );

    status = napi_create_reference ( env, constructor, 1, &WindowConstructor );
    assert ( status == napi_ok );
}
