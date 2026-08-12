# SQLite backed database plugin. Content shipped with the game and the
# player's saved state are both stored in databases so they can be queried
# together.
find_package(SQLite3)
if(SQLite3_FOUND)
  # FindSQLite3 only defines SQLite3::SQLite3 from CMake 4.3 on, before that the
  # imported target is named SQLite::SQLite3.
  if(NOT TARGET SQLite3::SQLite3 AND TARGET SQLite::SQLite3)
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
  endif()
  add_subdirectory(sqlite)
endif()
