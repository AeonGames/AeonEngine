set(NODES_HEADERS ModelComponent.h PointLight.h Camera.h)
set(NODES_SOURCES Plugin.cpp ModelComponent.cpp PointLight.cpp Camera.cpp)
add_library(AeonEngineComponents SHARED ${NODES_HEADERS} ${NODES_SOURCES})
if(MSVC)
  set_target_properties(
    AeonEngineComponents
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(AeonEngineComponents
                        PROPERTIES COMPILE_FLAGS
                                   "-Wall -DNOMINMAX -Werror -Wfatal-errors"
                                   PREFIX
                                   "")
endif()
target_link_libraries(AeonEngineComponents AeonEngine)
if(USE_CLANG_TIDY)
  set_target_properties(
    AeonEngineComponents
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
