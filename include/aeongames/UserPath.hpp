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
#ifndef AEONGAMES_USERPATH_H
#define AEONGAMES_USERPATH_H

#include "aeongames/Platform.hpp"
#include <filesystem>
#include <string>

namespace AeonGames
{
    /** @brief Which of the per-user locations a path is wanted for.
     *
     *  These are separate directories on Linux and merely a convention
     *  elsewhere, but keeping them apart everywhere is what allows a user to
     *  clear caches without losing saves, and a backup tool to skip the parts
     *  that are reproducible. */
    enum class UserDirectory
    {
        Data,   ///< Saves and live world state. Must be backed up.
        Config, ///< Settings the player changed. Small, worth backing up.
        Cache,  ///< Reproducible; safe to delete at any time.
    };

    /** @brief Application name used to build the per-user paths.
     *
     *  Defaults to "AeonEngine"; a game built on the engine sets its own so
     *  its saves do not collide with another's. */
    DLL void SetApplicationName ( const std::string& aName );
    /** @copydoc SetApplicationName */
    DLL const std::string& GetApplicationName();

    /** @brief Force every user directory under @p aPath, overriding the
     *         platform location. Pass an empty path to go back to the default. */
    DLL void SetUserPathOverride ( const std::filesystem::path& aPath );

    /** @brief Resolve a per-user directory, creating it if it does not exist.
     *
     *  The install directory is deliberately not used: it is read-only for a
     *  normal user under Program Files, /usr, an application bundle or a
     *  sandboxed package, and it is shared between accounts on one machine.
     *
     *  Resolution order:
     *    -# SetUserPathOverride, if one was set.
     *    -# The @c AEON_USER_PATH environment variable.
     *    -# A file named @c portable next to the executable, which selects
     *       @c <executable directory>/user for portable and development
     *       installs.
     *    -# The platform location: @c %%LOCALAPPDATA%%, the XDG base
     *       directories, or ~/Library on macOS.
     *
     *  @param aDirectory Which location is wanted.
     *  @return An existing directory the current user can write to. */
    DLL std::filesystem::path GetUserPath ( UserDirectory aDirectory );
}
#endif
