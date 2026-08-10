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

#include "aeongames/AeonEngine.hpp"
#include "aeongames/Database.hpp"
#include "aeongames/DatabaseSchema.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/UserPath.hpp"
#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace AeonGames
{
    namespace
    {
        // StringId only binds to a string literal array, not to a const char*.
        constexpr char kBackEnd[] = "SQLite";
        constexpr char kName[] = "schematest";

        class DatabaseSchemaTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                mRoot = std::filesystem::temp_directory_path() /
                        ( "aeon-schema-test-" + std::to_string ( ::testing::UnitTest::GetInstance()->random_seed() ) );
                std::filesystem::remove_all ( mRoot );
                std::filesystem::create_directories ( mRoot / "database" / kName );
                mResourcePath = GetResourcePath();
                SetUserPathOverride ( mRoot / "user" );
            }
            void TearDown() override
            {
                SetUserPathOverride ( {} );
                SetResourcePath ( mResourcePath );
                std::error_code error{};
                std::filesystem::remove_all ( mRoot, error );
            }
            void WriteStep ( uint32_t aVersion, const std::string& aStatements ) const
            {
                std::ostringstream name;
                name << std::setw ( 4 ) << std::setfill ( '0' ) << aVersion << ".sql";
                std::ofstream stream ( mRoot / "database" / kName / name.str(), std::ios::binary | std::ios::trunc );
                ASSERT_TRUE ( stream.good() );
                stream << aStatements;
            }
            /** A package indexes its directory when it is mounted, so every
                resource has to be on disk before this is called. */
            void Mount() const
            {
                SetResourcePath ( {mRoot.string() } );
            }
            std::unique_ptr<Database> OpenLoose ( const char* aFile ) const
            {
                std::unique_ptr<Database> database = ConstructDatabase ( StringId{kBackEnd} );
                database->Open ( ( mRoot / aFile ).string(), Database::OpenMode::ReadWriteCreate );
                return database;
            }
            std::filesystem::path mRoot{};
            std::vector<std::string> mResourcePath{};
        };
    }

    TEST_F ( DatabaseSchemaTest, WithoutStepsThereIsNothingToMigrate )
    {
        Mount();
        EXPECT_EQ ( GetDatabaseSchemaVersion ( kName ), 0u );
        std::unique_ptr<Database> database = OpenLoose ( "empty.db" );
        EXPECT_EQ ( MigrateDatabase ( *database, kName ), 0u );
        EXPECT_EQ ( database->GetSchemaVersion(), 0u );
    }

    TEST_F ( DatabaseSchemaTest, AppliesEveryStepInOrder )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT);" );
        WriteStep ( 2, "ALTER TABLE character ADD COLUMN faction TEXT;" );
        Mount();
        EXPECT_EQ ( GetDatabaseSchemaVersion ( kName ), 2u );

        std::unique_ptr<Database> database = OpenLoose ( "user.db" );
        EXPECT_EQ ( MigrateDatabase ( *database, kName ), 2u );
        EXPECT_EQ ( database->GetSchemaVersion(), 2u );
        database->Execute ( "INSERT INTO character (id, name, faction) VALUES (1, 'Knight', 'Fantasy')" );
    }

    /** A step numbered past the gap must not run: applying steps out of order
        would produce a schema no single version of the game ever described. */
    TEST_F ( DatabaseSchemaTest, StopsAtAGapInTheNumbering )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY);" );
        WriteStep ( 3, "CREATE TABLE unreachable (id INTEGER PRIMARY KEY);" );
        Mount();
        EXPECT_EQ ( GetDatabaseSchemaVersion ( kName ), 1u );

        std::unique_ptr<Database> database = OpenLoose ( "user.db" );
        EXPECT_EQ ( MigrateDatabase ( *database, kName ), 1u );
        EXPECT_THROW ( database->Prepare ( "SELECT 1 FROM unreachable" ), std::runtime_error );
    }

    TEST_F ( DatabaseSchemaTest, AppliesOnlyTheStepsNotYetSeen )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT);" );
        Mount();
        {
            std::unique_ptr<Database> database = OpenLoose ( "user.db" );
            EXPECT_EQ ( MigrateDatabase ( *database, kName ), 1u );
            database->Execute ( "INSERT INTO character VALUES (1, 'Aerin')" );
        }

        WriteStep ( 2, "ALTER TABLE character ADD COLUMN faction TEXT;" );
        Mount();

        std::unique_ptr<Database> database = OpenLoose ( "user.db" );
        EXPECT_EQ ( MigrateDatabase ( *database, kName ), 2u );
        std::unique_ptr<Database::Statement> select = database->Prepare ( "SELECT name, faction FROM character" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetText ( 0 ), "Aerin" );
        EXPECT_TRUE ( select->IsNull ( 1 ) );
    }

    /** Half a migration is worse than none: the player would be left on a
        version whose schema does not match what that version means. */
    TEST_F ( DatabaseSchemaTest, AFailedStepIsRolledBackWhole )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY);" );
        WriteStep ( 2, "CREATE TABLE half (id INTEGER PRIMARY KEY); THIS IS NOT SQL;" );
        Mount();

        std::unique_ptr<Database> database = OpenLoose ( "user.db" );
        EXPECT_THROW ( MigrateDatabase ( *database, kName ), std::runtime_error );
        EXPECT_EQ ( database->GetSchemaVersion(), 1u );
        EXPECT_THROW ( database->Prepare ( "SELECT 1 FROM half" ), std::runtime_error );

        // The connection is still usable, and a corrected step still applies.
        database->Execute ( "INSERT INTO character VALUES (1)" );
    }

    TEST_F ( DatabaseSchemaTest, RefusesADatabaseWrittenByANewerBuild )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY);" );
        Mount();

        std::unique_ptr<Database> database = OpenLoose ( "user.db" );
        database->SetSchemaVersion ( 7 );
        EXPECT_THROW ( MigrateDatabase ( *database, kName ), std::runtime_error );
        EXPECT_EQ ( database->GetSchemaVersion(), 7u );
    }

    TEST_F ( DatabaseSchemaTest, CreatesAndMigratesOnFirstRun )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT);" );
        Mount();

        const std::filesystem::path file = GetUserPath ( UserDirectory::Data ) / ( std::string{kName} + ".db" );
        ASSERT_FALSE ( std::filesystem::exists ( file ) );

        std::unique_ptr<Database> database = OpenUserDatabase ( StringId{kBackEnd}, kName );
        ASSERT_NE ( database, nullptr );
        EXPECT_TRUE ( std::filesystem::exists ( file ) );
        EXPECT_EQ ( database->GetSchemaVersion(), 1u );
        database->Execute ( "INSERT INTO character VALUES (1, 'Aerin')" );
    }

    TEST_F ( DatabaseSchemaTest, KeepsWhatThePlayerAlreadyHas )
    {
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT);" );
        Mount();
        {
            std::unique_ptr<Database> database = OpenUserDatabase ( StringId{kBackEnd}, kName );
            database->Execute ( "INSERT INTO character VALUES (1, 'Aerin')" );
        }

        std::unique_ptr<Database> database = OpenUserDatabase ( StringId{kBackEnd}, kName );
        std::unique_ptr<Database::Statement> select = database->Prepare ( "SELECT name FROM character" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetText ( 0 ), "Aerin" );
    }

    /** A starting world is far easier to ship as a prebuilt file than as insert
        statements, so first run copies one out of the package when it is there. */
    TEST_F ( DatabaseSchemaTest, SeedsFromThePackageOnFirstRun )
    {
        {
            std::unique_ptr<Database> seed = ConstructDatabase ( StringId{kBackEnd} );
            ASSERT_NE ( seed, nullptr );
            seed->Open ( ( mRoot / "database" / kName / "seed.db" ).string(),
                         Database::OpenMode::ReadWriteCreate );
            seed->Execute ( "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT)" );
            seed->Execute ( "INSERT INTO character VALUES (1, 'Starting NPC')" );
            seed->SetSchemaVersion ( 1 );
        }
        WriteStep ( 1, "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT);" );
        WriteStep ( 2, "ALTER TABLE character ADD COLUMN faction TEXT;" );
        Mount();

        std::unique_ptr<Database> database = OpenUserDatabase ( StringId{kBackEnd}, kName );
        ASSERT_NE ( database, nullptr );
        // Seeded at version 1, so only the second step should have been applied.
        EXPECT_EQ ( database->GetSchemaVersion(), 2u );
        std::unique_ptr<Database::Statement> select = database->Prepare ( "SELECT name, faction FROM character" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetText ( 0 ), "Starting NPC" );
        EXPECT_TRUE ( select->IsNull ( 1 ) );
    }

    TEST_F ( DatabaseSchemaTest, ReportsAnUnregisteredBackEnd )
    {
        Mount();
        EXPECT_THROW ( OpenUserDatabase ( StringId{"ThereIsNoSuchDatabase"}, kName ), std::runtime_error );
    }
}
