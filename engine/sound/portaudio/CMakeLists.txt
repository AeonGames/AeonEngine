include_directories(${PORTAUDIO_INCLUDE_DIR})
set(PORTAUDIO_HEADERS PortAudioSoundSystem.h)
set(PORTAUDIO_SOURCES PortAudioSoundSystem.cpp Plugin.cpp)
add_library(PortAudioSoundSystem
            SHARED
            ${PORTAUDIO_HEADERS}
            ${PORTAUDIO_SOURCES})
if(MSVC)
  set_target_properties(
    PortAudioSoundSystem
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(PortAudioSoundSystem
                        PROPERTIES COMPILE_FLAGS
                                   "-Wall -DNOMINMAX -Werror -Wfatal-errors"
                                   PREFIX
                                   "")
endif()
target_link_libraries(PortAudioSoundSystem AeonEngine ${PORTAUDIO_LIBRARY})

set_property(GLOBAL APPEND PROPERTY PLUGINS PortAudioSoundSystem)

if(USE_CLANG_TIDY)
  set_target_properties(
    PortAudioSoundSystem
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
