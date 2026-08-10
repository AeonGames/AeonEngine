# SQLite backed database plugin. Content shipped with the game and the
# player's saved state are both stored in databases so they can be queried
# together.
find_package(SQLite3)
if(SQLite3_FOUND)
  add_subdirectory(sqlite)
endif()
