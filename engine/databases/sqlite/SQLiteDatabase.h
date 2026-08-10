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
#ifndef AEONGAMES_SQLITEDATABASE_H
#define AEONGAMES_SQLITEDATABASE_H

#include "aeongames/Database.hpp"

struct sqlite3;

namespace AeonGames
{
    /** @brief SQLite backed Database implementation. */
    class SQLiteDatabase final : public Database
    {
    public:
        SQLiteDatabase();
        ~SQLiteDatabase() final;
        /** @name Overrides */
        ///@{
        void Open ( const std::string& aPath, OpenMode aMode ) final;
        void Close() final;
        void Execute ( const std::string& aStatement ) final;
        std::unique_ptr<Statement> Prepare ( const std::string& aStatement ) final;
        void Attach ( const std::string& aPath, const std::string& aName ) final;
        void Detach ( const std::string& aName ) final;
        uint32_t GetSchemaVersion() const final;
        void SetSchemaVersion ( uint32_t aVersion ) final;
        void Snapshot ( const std::string& aPath ) const final;
        ///@}
    private:
        sqlite3* mHandle{nullptr};
    };
}
#endif
