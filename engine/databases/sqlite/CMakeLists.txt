set(SQLITEDATABASE_HEADERS SQLiteDatabase.h)
set(SQLITEDATABASE_SOURCES SQLiteDatabase.cpp Plugin.cpp)
add_library(SQLiteDatabase SHARED ${SQLITEDATABASE_HEADERS}
                                  ${SQLITEDATABASE_SOURCES})
if(MSVC)
  set_target_properties(
    SQLiteDatabase
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(SQLiteDatabase PROPERTIES PREFIX "")
endif()
target_link_libraries(SQLiteDatabase AeonEngine SQLite3::SQLite3)

set_property(GLOBAL APPEND PROPERTY PLUGINS SQLiteDatabase)
