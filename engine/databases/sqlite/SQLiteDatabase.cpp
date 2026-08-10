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

#include "SQLiteDatabase.h"

#include <sqlite3.h>

#include <sstream>
#include <stdexcept>

namespace AeonGames
{
    namespace
    {
        [[noreturn]] void Fail ( sqlite3* aHandle, const std::string& aContext )
        {
            std::ostringstream stream;
            stream << aContext << ": "
                   << ( ( aHandle != nullptr ) ? sqlite3_errmsg ( aHandle ) : "out of memory" );
            throw std::runtime_error ( stream.str() );
        }

        /** SQLite has no parameter form for a database name, so an attached
            name has to be pasted into the statement; quote it rather than let a
            crafted name close the identifier and continue with its own SQL. */
        std::string QuoteIdentifier ( const std::string& aName )
        {
            std::string quoted{"\""};
            for ( char character : aName )
            {
                if ( character == '"' )
                {
                    quoted += '"';
                }
                quoted += character;
            }
            quoted += '"';
            return quoted;
        }

        std::string QuoteLiteral ( const std::string& aValue )
        {
            std::string quoted{"'"};
            for ( char character : aValue )
            {
                if ( character == '\'' )
                {
                    quoted += '\'';
                }
                quoted += character;
            }
            quoted += '\'';
            return quoted;
        }

        class SQLiteStatement final : public Database::Statement
        {
        public:
            SQLiteStatement ( sqlite3* aHandle, const std::string& aStatement ) : mDatabase{aHandle}
            {
                if ( sqlite3_prepare_v2 ( mDatabase, aStatement.c_str(), -1, &mHandle, nullptr ) != SQLITE_OK )
                {
                    Fail ( mDatabase, "Unable to prepare '" + aStatement + "'" );
                }
            }
            ~SQLiteStatement() final
            {
                sqlite3_finalize ( mHandle );
            }
            void Bind ( int32_t aIndex, int64_t aValue ) final
            {
                sqlite3_bind_int64 ( mHandle, aIndex, aValue );
            }
            void Bind ( int32_t aIndex, double aValue ) final
            {
                sqlite3_bind_double ( mHandle, aIndex, aValue );
            }
            void Bind ( int32_t aIndex, const std::string& aValue ) final
            {
                sqlite3_bind_text ( mHandle, aIndex, aValue.c_str(), -1, SQLITE_TRANSIENT );
            }
            void BindNull ( int32_t aIndex ) final
            {
                sqlite3_bind_null ( mHandle, aIndex );
            }
            bool Step() final
            {
                const int result = sqlite3_step ( mHandle );
                if ( result == SQLITE_ROW )
                {
                    return true;
                }
                if ( result != SQLITE_DONE )
                {
                    Fail ( mDatabase, "Statement failed" );
                }
                return false;
            }
            void Reset() final
            {
                sqlite3_reset ( mHandle );
                sqlite3_clear_bindings ( mHandle );
            }
            int64_t GetInt ( int32_t aColumn ) const final
            {
                return sqlite3_column_int64 ( mHandle, aColumn );
            }
            double GetDouble ( int32_t aColumn ) const final
            {
                return sqlite3_column_double ( mHandle, aColumn );
            }
            std::string GetText ( int32_t aColumn ) const final
            {
                const unsigned char* text = sqlite3_column_text ( mHandle, aColumn );
                return ( text != nullptr ) ? reinterpret_cast<const char*> ( text ) : std::string{};
            }
            bool IsNull ( int32_t aColumn ) const final
            {
                return sqlite3_column_type ( mHandle, aColumn ) == SQLITE_NULL;
            }
            int32_t GetColumnCount() const final
            {
                return sqlite3_column_count ( mHandle );
            }
        private:
            sqlite3* mDatabase{nullptr};
            sqlite3_stmt* mHandle{nullptr};
        };
    }

    SQLiteDatabase::SQLiteDatabase() = default;

    SQLiteDatabase::~SQLiteDatabase()
    {
        SQLiteDatabase::Close();
    }

    void SQLiteDatabase::Open ( const std::string& aPath, OpenMode aMode )
    {
        Close();
        int flags = SQLITE_OPEN_READONLY;
        switch ( aMode )
        {
        case OpenMode::ReadWrite:
            flags = SQLITE_OPEN_READWRITE;
            break;
        case OpenMode::ReadWriteCreate:
            flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
            break;
        case OpenMode::ReadOnly:
        default:
            break;
        }
        if ( sqlite3_open_v2 ( aPath.c_str(), &mHandle, flags, nullptr ) != SQLITE_OK )
        {
            const std::string message = ( mHandle != nullptr ) ? sqlite3_errmsg ( mHandle ) : "out of memory";
            sqlite3_close ( mHandle );
            mHandle = nullptr;
            throw std::runtime_error ( "Unable to open " + aPath + ": " + message );
        }
        sqlite3_extended_result_codes ( mHandle, 1 );
        if ( aMode != OpenMode::ReadOnly )
        {
            // Write-ahead logging survives a crash mid-write far better than the
            // rollback journal, which matters most for the file holding saves.
            Execute ( "PRAGMA journal_mode = WAL" );
            Execute ( "PRAGMA synchronous = NORMAL" );
        }
        Execute ( "PRAGMA foreign_keys = ON" );
    }

    void SQLiteDatabase::Close()
    {
        sqlite3_close ( mHandle );
        mHandle = nullptr;
    }

    void SQLiteDatabase::Execute ( const std::string& aStatement )
    {
        char* message = nullptr;
        if ( sqlite3_exec ( mHandle, aStatement.c_str(), nullptr, nullptr, &message ) != SQLITE_OK )
        {
            std::ostringstream stream;
            stream << "Statement failed: " << ( ( message != nullptr ) ? message : "unknown error" );
            sqlite3_free ( message );
            throw std::runtime_error ( stream.str() );
        }
    }

    std::unique_ptr<Database::Statement> SQLiteDatabase::Prepare ( const std::string& aStatement )
    {
        return std::make_unique<SQLiteStatement> ( mHandle, aStatement );
    }

    void SQLiteDatabase::Attach ( const std::string& aPath, const std::string& aName )
    {
        Execute ( "ATTACH DATABASE " + QuoteLiteral ( aPath ) + " AS " + QuoteIdentifier ( aName ) );
    }

    void SQLiteDatabase::Detach ( const std::string& aName )
    {
        Execute ( "DETACH DATABASE " + QuoteIdentifier ( aName ) );
    }

    uint32_t SQLiteDatabase::GetSchemaVersion() const
    {
        sqlite3_stmt* statement = nullptr;
        if ( sqlite3_prepare_v2 ( mHandle, "PRAGMA user_version", -1, &statement, nullptr ) != SQLITE_OK )
        {
            Fail ( mHandle, "Unable to read the schema version" );
        }
        const uint32_t version = ( sqlite3_step ( statement ) == SQLITE_ROW ) ?
                                 static_cast<uint32_t> ( sqlite3_column_int64 ( statement, 0 ) ) : 0;
        sqlite3_finalize ( statement );
        return version;
    }

    void SQLiteDatabase::SetSchemaVersion ( uint32_t aVersion )
    {
        // PRAGMA takes no bound parameters, hence the built statement.
        Execute ( "PRAGMA user_version = " + std::to_string ( aVersion ) );
    }

    void SQLiteDatabase::Snapshot ( const std::string& aPath ) const
    {
        // VACUUM INTO writes the whole database in one step and leaves nothing
        // behind if it fails, which is what a save slot needs.
        char* message = nullptr;
        const std::string statement = "VACUUM INTO " + QuoteLiteral ( aPath );
        if ( sqlite3_exec ( mHandle, statement.c_str(), nullptr, nullptr, &message ) != SQLITE_OK )
        {
            std::ostringstream stream;
            stream << "Unable to snapshot into " << aPath << ": "
                   << ( ( message != nullptr ) ? message : "unknown error" );
            sqlite3_free ( message );
            throw std::runtime_error ( stream.str() );
        }
    }
}
