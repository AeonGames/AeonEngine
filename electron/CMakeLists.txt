include(electron)

set(MODULE_SRCS
    Module.cpp
    Window.h
    Window.cpp
    Scene.h
    Scene.cpp
    WinDelayLoadHook.cpp
)

set(JS_SOURCES
    main.js
    index.html
    package.json
)

add_custom_target(copy-electron-dependencies
    COMMAND ${CMAKE_COMMAND} -E copy ${JS_SOURCES} ${ELECTRON_BINARY_DIR}/resources/app/
    DEPENDS
        ${JS_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Copying Electron Sources"
    COMMAND_EXPAND_LISTS
)

add_library(AeonEngineModule SHARED ${MODULE_SRCS} ${JS_SOURCES_OUTPUT})
set_target_properties(AeonEngineModule PROPERTIES SUFFIX ".node")
set_target_properties(AeonEngineModule PROPERTIES PREFIX "")
set_target_properties(AeonEngineModule PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ELECTRON_BINARY_DIR}/resources/app/node_modules")
set_target_properties(AeonEngineModule PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${ELECTRON_BINARY_DIR}/resources/app/node_modules")
target_link_libraries(AeonEngineModule AeonEngine ${NODE_LIBRARY})
target_include_directories(AeonEngineModule PUBLIC ${CMAKE_SOURCE_DIR}/include ${NODE_INCLUDE_DIR})
add_dependencies(AeonEngineModule copy-electron-dependencies)