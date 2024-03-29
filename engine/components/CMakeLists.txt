set(NODES_HEADERS ModelComponent.h PointLight.h Camera.h)
set(NODES_SOURCES Plugin.cpp ModelComponent.cpp PointLight.cpp Camera.cpp)
add_library(AeonEngineComponents SHARED ${NODES_HEADERS} ${NODES_SOURCES})
if(MSVC)
  set_target_properties(
    AeonEngineComponents
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(AeonEngineComponents PROPERTIES PREFIX "")
endif()
target_link_libraries(AeonEngineComponents AeonEngine)

set_property(GLOBAL APPEND PROPERTY PLUGINS AeonEngineComponents)

if(USE_CLANG_TIDY)
  set_target_properties(
    AeonEngineComponents
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
if(IWYU_PROGRAM)
  set_property(TARGET AeonEngineComponents PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_COMMAND_LINE})
endif()
