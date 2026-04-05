set(AEONGUI_PLUGIN_HEADERS
    AeonGuiOverlay.hpp)

set(AEONGUI_PLUGIN_SOURCES
    AeonGuiOverlay.cpp
    Plugin.cpp)

add_library(AeonGUIPlugin
            SHARED
            ${AEONGUI_PLUGIN_HEADERS}
            ${AEONGUI_PLUGIN_SOURCES}
            ${CMAKE_SOURCE_DIR}/include/aeongames/GuiOverlay.hpp)

target_link_libraries(AeonGUIPlugin
                      AeonGUI
                      AeonEngine)

if(MSVC)
  set_target_properties(
    AeonGUIPlugin
    PROPERTIES COMPILE_FLAGS "-WX -D_CRT_SECURE_NO_WARNINGS")
elseif(MINGW OR MSYS)
  set_target_properties(AeonGUIPlugin PROPERTIES PREFIX "")
endif()

set_property(GLOBAL APPEND PROPERTY PLUGINS AeonGUIPlugin)
