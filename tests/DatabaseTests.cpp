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

#include "aeongames/Database.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/UserPath.hpp"
#include "gtest/gtest.h"

#include <cstdlib>
#include <filesystem>

namespace AeonGames
{
    namespace
    {
        class DatabaseTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                mRoot = std::filesystem::temp_directory_path() /
                        ( "aeon-database-test-" + std::to_string ( ::testing::UnitTest::GetInstance()->random_seed() ) );
                std::filesystem::remove_all ( mRoot );
                std::filesystem::create_directories ( mRoot );
            }
            void TearDown() override
            {
                std::error_code error{};
                std::filesystem::remove_all ( mRoot, error );
            }
            std::string Path ( const char* aName ) const
            {
                return ( mRoot / aName ).string();
            }
            std::filesystem::path mRoot{};
        };
    }

    TEST_F ( DatabaseTest, UnknownBackEndConstructsNothing )
    {
        EXPECT_EQ ( ConstructDatabase ( StringId{"ThereIsNoSuchDatabase"} ), nullptr );
    }

    TEST_F ( DatabaseTest, RoundTripsValuesThroughAStatement )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        database->Execute ( "CREATE TABLE part (id INTEGER PRIMARY KEY, name TEXT, weight REAL, note TEXT)" );

        std::unique_ptr<Database::Statement> insert =
            database->Prepare ( "INSERT INTO part (id, name, weight, note) VALUES (?1, ?2, ?3, ?4)" );
        insert->Bind ( 1, static_cast<int64_t> ( 7 ) );
        insert->Bind ( 2, std::string{"Helmet"} );
        insert->Bind ( 3, 2.5 );
        insert->BindNull ( 4 );
        EXPECT_FALSE ( insert->Step() );
        insert->Reset();
        insert->Bind ( 1, static_cast<int64_t> ( 8 ) );
        insert->Bind ( 2, std::string{"Cape"} );
        insert->Bind ( 3, 0.25 );
        insert->Bind ( 4, std::string{"flows"} );
        EXPECT_FALSE ( insert->Step() );

        std::unique_ptr<Database::Statement> select =
            database->Prepare ( "SELECT id, name, weight, note FROM part ORDER BY id" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetColumnCount(), 4 );
        EXPECT_EQ ( select->GetInt ( 0 ), 7 );
        EXPECT_EQ ( select->GetText ( 1 ), "Helmet" );
        EXPECT_DOUBLE_EQ ( select->GetDouble ( 2 ), 2.5 );
        EXPECT_TRUE ( select->IsNull ( 3 ) );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetInt ( 0 ), 8 );
        EXPECT_EQ ( select->GetText ( 1 ), "Cape" );
        EXPECT_FALSE ( select->IsNull ( 3 ) );
        EXPECT_FALSE ( select->Step() );
    }

    /** Content shipped with the game and the player's own state live in
        separate files so a patch can replace one without touching the other.
        Being able to query across the two is what makes that split workable. */
    TEST_F ( DatabaseTest, JoinsAcrossAnAttachedDatabase )
    {
        {
            std::unique_ptr<Database> content = ConstructDatabase ( StringId{"SQLite"} );
            ASSERT_NE ( content, nullptr );
            content->Open ( Path ( "content.db" ), Database::OpenMode::ReadWriteCreate );
            content->Execute ( "CREATE TABLE outfit (id INTEGER PRIMARY KEY, name TEXT)" );
            content->Execute ( "INSERT INTO outfit VALUES (1, 'Knight'), (2, 'Pirate')" );
        }

        std::unique_ptr<Database> user = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( user, nullptr );
        user->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        user->Execute ( "CREATE TABLE character (id INTEGER PRIMARY KEY, outfit INTEGER)" );
        user->Execute ( "INSERT INTO character VALUES (1, 2)" );
        user->Attach ( Path ( "content.db" ), "content" );

        std::unique_ptr<Database::Statement> select = user->Prepare (
                "SELECT outfit.name FROM character "
                "JOIN content.outfit AS outfit ON outfit.id = character.outfit" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetText ( 0 ), "Pirate" );
        select.reset();

        user->Detach ( "content" );
        EXPECT_THROW ( user->Execute ( "SELECT 1 FROM content.outfit" ), std::runtime_error );
    }

    TEST_F ( DatabaseTest, RemembersTheSchemaVersion )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        EXPECT_EQ ( database->GetSchemaVersion(), 0u );
        database->SetSchemaVersion ( 3 );
        EXPECT_EQ ( database->GetSchemaVersion(), 3u );
        database->Close();

        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadOnly );
        EXPECT_EQ ( database->GetSchemaVersion(), 3u );
    }

    TEST_F ( DatabaseTest, SnapshotCopiesTheWholeDatabase )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        database->Execute ( "CREATE TABLE character (id INTEGER PRIMARY KEY, name TEXT)" );
        database->Execute ( "INSERT INTO character VALUES (1, 'Aerin')" );
        database->SetSchemaVersion ( 2 );
        database->Snapshot ( Path ( "slot1.db" ) );
        database->Execute ( "UPDATE character SET name = 'Knight' WHERE id = 1" );

        std::unique_ptr<Database> slot = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( slot, nullptr );
        slot->Open ( Path ( "slot1.db" ), Database::OpenMode::ReadOnly );
        EXPECT_EQ ( slot->GetSchemaVersion(), 2u );
        std::unique_ptr<Database::Statement> select = slot->Prepare ( "SELECT name FROM character" );
        ASSERT_TRUE ( select->Step() );
        EXPECT_EQ ( select->GetText ( 0 ), "Aerin" );
    }

    TEST_F ( DatabaseTest, ReadOnlyRefusesToCreateOrWrite )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        EXPECT_THROW ( database->Open ( Path ( "missing.db" ), Database::OpenMode::ReadOnly ), std::runtime_error );

        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        database->Execute ( "CREATE TABLE character (id INTEGER PRIMARY KEY)" );
        database->Close();

        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadOnly );
        EXPECT_THROW ( database->Execute ( "INSERT INTO character VALUES (1)" ), std::runtime_error );
    }

    TEST_F ( DatabaseTest, ReportsFailuresInsteadOfSilentlyContinuing )
    {
        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        database->Open ( Path ( "user.db" ), Database::OpenMode::ReadWriteCreate );
        EXPECT_THROW ( database->Execute ( "THIS IS NOT SQL" ), std::runtime_error );
        EXPECT_THROW ( database->Prepare ( "SELECT * FROM no_such_table" ), std::runtime_error );
    }

    TEST_F ( DatabaseTest, UserPathOverrideSeparatesTheDirectoryKinds )
    {
        const std::filesystem::path root = mRoot / "user";
        SetUserPathOverride ( root );
        const std::filesystem::path data = GetUserPath ( UserDirectory::Data );
        const std::filesystem::path config = GetUserPath ( UserDirectory::Config );
        const std::filesystem::path cache = GetUserPath ( UserDirectory::Cache );
        SetUserPathOverride ( {} );

        EXPECT_TRUE ( std::filesystem::is_directory ( data ) );
        EXPECT_TRUE ( std::filesystem::is_directory ( config ) );
        EXPECT_TRUE ( std::filesystem::is_directory ( cache ) );
        EXPECT_NE ( data, config );
        EXPECT_NE ( data, cache );
        EXPECT_NE ( config, cache );
        EXPECT_EQ ( data.parent_path(), root );
    }

    /** The install tree is read-only for a normal user on every supported
        platform, so a writable location has to come from the user's profile. */
    TEST_F ( DatabaseTest, UserPathIsWritableAndUnderTheApplicationName )
    {
        SetApplicationName ( "AeonEngineDatabaseTest" );
        const std::filesystem::path data = GetUserPath ( UserDirectory::Data );
        SetApplicationName ( "AeonEngine" );

        ASSERT_TRUE ( std::filesystem::is_directory ( data ) );
        EXPECT_NE ( data.string().find ( "AeonEngineDatabaseTest" ), std::string::npos );

        std::unique_ptr<Database> database = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( database, nullptr );
        const std::filesystem::path file = data / "user.db";
        database->Open ( file.string(), Database::OpenMode::ReadWriteCreate );
        database->Execute ( "CREATE TABLE IF NOT EXISTS probe (id INTEGER PRIMARY KEY)" );
        database->Close();

        std::error_code error{};
        std::filesystem::remove_all ( data, error );
    }
}
