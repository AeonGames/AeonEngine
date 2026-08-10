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

#include "aeongames/DatabaseSchema.hpp"

#include "aeongames/AeonEngine.hpp"
#include "aeongames/Database.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/UserPath.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace AeonGames
{
    namespace
    {
        std::string StepResource ( const std::string& aName, uint32_t aVersion )
        {
            std::ostringstream stream;
            stream << "database/" << aName << '/'
                   << std::setw ( 4 ) << std::setfill ( '0' ) << aVersion << ".sql";
            return stream.str();
        }

        std::string SeedResource ( const std::string& aName )
        {
            return "database/" + aName + "/seed.db";
        }

        std::string LoadText ( const std::string& aResource, size_t aSize )
        {
            std::string text ( aSize, '\0' );
            LoadResource ( aResource, text.data(), aSize );
            return text;
        }

        void WriteSeed ( const std::string& aName, const std::filesystem::path& aFile )
        {
            const std::string resource = SeedResource ( aName );
            const size_t size = GetResourceSize ( resource );
            if ( size == 0 )
            {
                return;
            }
            std::vector<char> buffer ( size );
            LoadResource ( resource, buffer.data(), size );
            std::ofstream stream ( aFile, std::ios::binary | std::ios::trunc );
            if ( !stream )
            {
                throw std::runtime_error ( "Unable to create " + aFile.string() );
            }
            stream.write ( buffer.data(), static_cast<std::streamsize> ( buffer.size() ) );
            if ( !stream )
            {
                throw std::runtime_error ( "Unable to write " + aFile.string() );
            }
        }
    }

    uint32_t GetDatabaseSchemaVersion ( const std::string& aName )
    {
        uint32_t version = 0;
        while ( GetResourceSize ( StepResource ( aName, version + 1 ) ) != 0 )
        {
            ++version;
        }
        return version;
    }

    uint32_t MigrateDatabase ( Database& aDatabase, const std::string& aName )
    {
        const uint32_t available = GetDatabaseSchemaVersion ( aName );
        uint32_t current = aDatabase.GetSchemaVersion();
        if ( current > available )
        {
            std::ostringstream stream;
            stream << "The " << aName << " database is at schema version " << current
                   << " but this build only knows up to " << available
                   << "; it was written by a newer version.";
            throw std::runtime_error ( stream.str() );
        }
        while ( current < available )
        {
            const uint32_t next = current + 1;
            const std::string resource = StepResource ( aName, next );
            const std::string statements = LoadText ( resource, GetResourceSize ( resource ) );
            aDatabase.Execute ( "BEGIN" );
            try
            {
                aDatabase.Execute ( statements );
                aDatabase.SetSchemaVersion ( next );
            }
            catch ( ... )
            {
                try
                {
                    aDatabase.Execute ( "ROLLBACK" );
                }
                catch ( const std::runtime_error& )
                {
                    // Report why the migration failed, not why undoing it did.
                }
                throw;
            }
            aDatabase.Execute ( "COMMIT" );
            current = next;
        }
        return current;
    }

    std::unique_ptr<Database> OpenUserDatabase ( const StringId& aBackEnd, const std::string& aName )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( aBackEnd );
        if ( database == nullptr )
        {
            throw std::runtime_error ( "No database back-end is registered as " + std::string{aBackEnd.GetString() } );
        }
        const std::filesystem::path file = GetUserPath ( UserDirectory::Data ) / ( aName + ".db" );
        std::error_code error{};
        if ( !std::filesystem::exists ( file, error ) )
        {
            WriteSeed ( aName, file );
        }
        database->Open ( file.string(), Database::OpenMode::ReadWriteCreate );
        MigrateDatabase ( *database, aName );
        return database;
    }
}
