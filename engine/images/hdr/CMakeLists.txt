set(RADIANCEIMAGE_HEADERS RadianceImage.h)
set(RADIANCEIMAGE_SOURCES RadianceImage.cpp Plugin.cpp)
add_library(RadianceImage SHARED ${RADIANCEIMAGE_HEADERS} ${RADIANCEIMAGE_SOURCES})
if(MSVC)
  set_target_properties(
    RadianceImage
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(RadianceImage PROPERTIES PREFIX "")
endif()
target_link_libraries(RadianceImage AeonEngine)

set_property(GLOBAL APPEND PROPERTY PLUGINS RadianceImage)

if(USE_CLANG_TIDY)
  set_target_properties(
    RadianceImage
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
