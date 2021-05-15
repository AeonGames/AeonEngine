include_directories(${VORBIS_INCLUDE_DIR} ${OGG_INCLUDE_DIR})
set(OGGVORBIS_HEADERS OggSound.h)
set(OGGVORBIS_SOURCES OggSound.cpp Plugin.cpp)
add_library(OggSound SHARED ${OGGVORBIS_HEADERS} ${OGGVORBIS_SOURCES})
if(MSVC)
  set_target_properties(
    OggSound
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(OggSound
                        PROPERTIES 
                          PREFIX
                            ""
  )
endif()
target_link_libraries(OggSound AeonEngine ${OGGVORBIS_LIBRARIES})

set_property(GLOBAL APPEND PROPERTY PLUGINS OggSound)

if(USE_CLANG_TIDY)
  set_target_properties(
    OggSound
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
