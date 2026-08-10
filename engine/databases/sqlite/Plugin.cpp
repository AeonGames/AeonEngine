/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the SQLite database plugin.*/
#include "aeongames/Platform.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/Database.hpp"
#include "aeongames/StringId.hpp"
#include "SQLiteDatabase.h"

extern "C"
{
    bool SQLiteStartUp()
    {
        return AeonGames::RegisterDatabaseConstructor ( "SQLite",
                []()
        {
            return std::make_unique<AeonGames::SQLiteDatabase>();
        } );
    }

    void SQLiteShutdown()
    {
        AeonGames::UnregisterDatabaseConstructor ( "SQLite" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "SQLite database",
        "Implements SQLite backed database support",
        SQLiteStartUp,
        SQLiteShutdown
    };
}
