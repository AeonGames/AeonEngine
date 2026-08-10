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
#ifndef AEONGAMES_DATABASESCHEMA_H
#define AEONGAMES_DATABASESCHEMA_H

#include "aeongames/Platform.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace AeonGames
{
    class Database;
    class StringId;

    /** @name Schema migration
     *
     *  Once a player has a save it can only be migrated, never recreated, so
     *  the steps that take a database from one version to the next are assets
     *  rather than code: for a database named @c user they are the resources
     *
     *      database/user/0001.sql
     *      database/user/0002.sql
     *
     *  read through the ordinary resource path. The number in the file name is
     *  the schema version it produces, so a fresh database is simply one at
     *  version zero that has every step applied to it, and changing the schema
     *  means adding an asset rather than editing the engine.
     *
     *  A step is applied in a transaction together with the version stamp, so a
     *  failed migration leaves the database on the version it started at rather
     *  than half converted.
     */
    /*@{*/
    /** @brief Highest schema version the currently mounted resources can produce.
     *  @param aName Logical database name, @c user for @c database/user/NNNN.sql.
     *  @return The last consecutively numbered step found, or zero when there are none. */
    DLL uint32_t GetDatabaseSchemaVersion ( const std::string& aName );

    /** @brief Apply every step @p aDatabase has not yet seen.
     *  @param aDatabase An open, writable database.
     *  @param aName Logical database name.
     *  @return The schema version the database is on afterwards.
     *  @throws std::runtime_error when a step fails, or when the database was
     *          written by a build with a newer schema than the one available;
     *          downgrading would have to discard the player's data. */
    DLL uint32_t MigrateDatabase ( Database& aDatabase, const std::string& aName );

    /** @brief Open the current user's copy of a database, creating it on first run.
     *
     *  The file lives in GetUserPath ( UserDirectory::Data ) as @c <aName>.db.
     *  When it does not exist yet and a @c database/<aName>/seed.db resource is
     *  present, that resource is written out first, which is how a game ships a
     *  starting world rather than expressing one as insert statements. The
     *  database is migrated to the current schema version either way.
     *
     *  @param aBackEnd Identifier of a registered Database back-end.
     *  @param aName Logical database name.
     *  @return A unique_ptr owning the open database.
     *  @throws std::runtime_error when the back-end is not registered, or when
     *          the database cannot be created, seeded or migrated. */
    DLL std::unique_ptr<Database> OpenUserDatabase ( const StringId& aBackEnd, const std::string& aName );
    /*@}*/
}
#endif
