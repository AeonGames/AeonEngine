set(NODES_HEADERS
)
set(NODES_SOURCES
  Plugin.cpp
)

add_library(TurnbasedTactics SHARED ${NODES_HEADERS} ${NODES_SOURCES})

if(MSVC)
  set_target_properties(
    TurnbasedTactics
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(TurnbasedTactics PROPERTIES PREFIX "")
endif()
target_link_libraries(TurnbasedTactics AeonEngine)

set_property(GLOBAL APPEND PROPERTY PLUGINS TurnbasedTactics)

if(USE_CLANG_TIDY)
  set_target_properties(
    TurnbasedTactics
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()
if(IWYU_PROGRAM)
  set_property(TARGET TurnbasedTactics PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_COMMAND_LINE})
endif()
