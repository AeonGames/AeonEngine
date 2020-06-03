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
#include "aeongames/Scene.h"
#include "Scene.h"

static napi_ref SceneConstructor{};
void InitializeScene ( napi_env env, napi_value exports )
{
    napi_value constructor = nullptr;
    napi_status status = napi_define_class ( env,
                         "Scene",
                         NAPI_AUTO_LENGTH,
                         Scene,
                         nullptr,
                         0,
                         nullptr,
                         &constructor );
    assert ( status == napi_ok );
    status = napi_set_named_property ( env, exports, "Scene", constructor );
    assert ( status == napi_ok );

    status = napi_create_reference ( env, constructor, 1, &SceneConstructor );
    assert ( status == napi_ok );
}

napi_value Scene ( napi_env env, napi_callback_info info )
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
