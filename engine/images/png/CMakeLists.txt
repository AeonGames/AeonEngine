set(PNGIMAGE_HEADERS PngImage.h)
set(PNGIMAGE_SOURCES PngImage.cpp Plugin.cpp)
add_library(PngImage SHARED ${PNGIMAGE_HEADERS} ${PNGIMAGE_SOURCES})
if(MSVC)
  set_target_properties(
    PngImage
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(PngImage
                        PROPERTIES COMPILE_FLAGS
                                   "-Wall -DNOMINMAX -Werror -Wfatal-errors"
                                   PREFIX
                                   "")
endif()
target_link_libraries(PngImage AeonEngine ${PNG_LIBRARIES})

set_property(GLOBAL APPEND PROPERTY PLUGINS PngImage)

if(USE_CLANG_TIDY)
  set_target_properties(
    PngImage
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
