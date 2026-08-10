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
#ifndef AEONGAMES_DATABASE_H
#define AEONGAMES_DATABASE_H

#include "aeongames/Platform.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace AeonGames
{
    class StringId;

    /** @brief Abstract interface for a back-end holding persistent structured data.
     *
     *  Content shipped with the game and the player's own saved state both live
     *  in a database rather than in bespoke files, so the two can be queried
     *  together: attach the read-only content database alongside the writable
     *  save and join across them.
     *
     *  The interface is deliberately statement oriented rather than an object
     *  mapper. Back-ends are expected to speak SQL, which is what makes tables
     *  from different sources combinable without the engine knowing their shape;
     *  the abstraction exists to swap one SQL engine for another, not to hide
     *  the query language.
     */
    class Database
    {
    public:
        /** @brief How a database file is opened. */
        enum class OpenMode
        {
            ReadOnly,        ///< Fails when the file does not exist; never written to.
            ReadWrite,       ///< Fails when the file does not exist.
            ReadWriteCreate, ///< Creates the file when it does not exist.
        };

        /** @brief A compiled statement, bound and stepped to read or write rows.
         *
         *  Parameter indices are one based, column indices zero based, matching
         *  the convention of every SQL back-end this is likely to wrap. */
        class Statement
        {
        public:
            /** @brief Virtual destructor. */
            virtual ~Statement() = default;
            /**@name Parameter binding */
            /*@{*/
            virtual void Bind ( int32_t aIndex, int64_t aValue ) = 0;
            virtual void Bind ( int32_t aIndex, double aValue ) = 0;
            virtual void Bind ( int32_t aIndex, const std::string& aValue ) = 0;
            virtual void BindNull ( int32_t aIndex ) = 0;
            /*@}*/
            /** @brief Advance to the next row.
             *  @return true when a row is available, false once the statement is done. */
            virtual bool Step() = 0;
            /** @brief Return to the start, clearing bound parameters, so the
             *         statement can be run again without recompiling it. */
            virtual void Reset() = 0;
            /**@name Column access, valid while Step() last returned true */
            /*@{*/
            virtual int64_t GetInt ( int32_t aColumn ) const = 0;
            virtual double GetDouble ( int32_t aColumn ) const = 0;
            virtual std::string GetText ( int32_t aColumn ) const = 0;
            virtual bool IsNull ( int32_t aColumn ) const = 0;
            virtual int32_t GetColumnCount() const = 0;
            /*@}*/
        };

        /** @brief Virtual destructor. */
        virtual ~Database() = default;
        /** @brief Open a database file.
         *  @param aPath Path to the file on disk.
         *  @param aMode How the file should be opened.
         *  @throws std::runtime_error when the file cannot be opened. */
        virtual void Open ( const std::string& aPath, OpenMode aMode ) = 0;
        /** @brief Close the database, discarding any prepared statements. */
        virtual void Close() = 0;
        /** @brief Run one or more statements that return no rows.
         *  @throws std::runtime_error on failure. */
        virtual void Execute ( const std::string& aStatement ) = 0;
        /** @brief Compile a statement for repeated binding and stepping.
         *  @throws std::runtime_error when the statement does not compile. */
        virtual std::unique_ptr<Statement> Prepare ( const std::string& aStatement ) = 0;
        /** @brief Make another database file visible under @p aName, so queries
         *         can join across both. */
        virtual void Attach ( const std::string& aPath, const std::string& aName ) = 0;
        /** @brief Drop a database previously made visible by Attach. */
        virtual void Detach ( const std::string& aName ) = 0;
        /** @brief Schema version stamped into the file, used to decide whether a
         *         save written by an older build needs migrating. */
        virtual uint32_t GetSchemaVersion() const = 0;
        /** @brief Stamp the schema version into the file. */
        virtual void SetSchemaVersion ( uint32_t aVersion ) = 0;
        /** @brief Write a consistent copy of the database to @p aPath.
         *
         *  Back-ends are expected to make this atomic, which is what lets a save
         *  slot be written without a half-copied file surviving a crash. */
        virtual void Snapshot ( const std::string& aPath ) const = 0;
    };

    /**@name Factory Functions */
    /*@{*/
    /** @brief Construct a Database back-end for the given identifier.
     *  @param aIdentifier Identifier selecting the back-end implementation.
     *  @return A unique_ptr owning the newly created Database, or nullptr when
     *          no back-end is registered under that identifier. */
    DLL std::unique_ptr<Database> ConstructDatabase ( const StringId& aIdentifier );
    /** @copydoc ConstructDatabase(const StringId&) */
    DLL std::unique_ptr<Database> ConstructDatabase ( const std::string& aIdentifier );
    /** @copydoc ConstructDatabase(const StringId&) */
    DLL std::unique_ptr<Database> ConstructDatabase ( uint32_t aIdentifier );
    /** Registers a Database back-end for a specific identifier.*/
    DLL bool RegisterDatabaseConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Database>() >& aConstructor );
    /** Unregisters a Database back-end for a specific identifier.*/
    DLL bool UnregisterDatabaseConstructor ( const StringId& aIdentifier );
    /** Enumerates Database back-end identifiers via an enumerator functor.*/
    DLL void EnumerateDatabaseConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** @brief Names of every registered Database back-end. */
    DLL std::vector<std::string> GetDatabaseConstructorNames();
    /*@}*/
}
#endif
