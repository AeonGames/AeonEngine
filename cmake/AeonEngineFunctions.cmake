# Copyright (C) 2017 AeonGames, Rodrigo Jose Hernandez Cordoba
# Licensed under the terms of the Apache 2.0 License.

function(generate_compile_commands target sources)
    if(MSVC AND CMAKE_EXPORT_COMPILE_COMMANDS)
        get_target_property(INCLUDE_DIRECTORIES ${target} INCLUDE_DIRECTORIES)
        foreach(INCLUDE_DIRECTORY ${INCLUDE_DIRECTORIES})
            set(INCLUDE_FLAGS "${INCLUDE_FLAGS} -I${INCLUDE_DIRECTORY}")
        endforeach()
        file(WRITE ${CMAKE_BINARY_DIR}/compile_commands.json "[\n")
        foreach(SOURCE_FILE ${sources})
            get_source_file_property(LANGUAGE ${SOURCE_FILE} LANGUAGE)
            if(LANGUAGE STREQUAL "CXX" OR LANGUAGE STREQUAL "C")
                file(APPEND ${CMAKE_BINARY_DIR}/compile_commands.json "{\n\t\"directory\": \"${CMAKE_CURRENT_SOURCE_DIR}\",\n\t\"command\": \"cl.exe -c ${INCLUDE_FLAGS} -fms-extensions -fms-compatibility -fms-compatibility-version=19 -std=c++14 -D_MSC_VER=1900\",\n\t\"file\": \"${SOURCE_FILE}\"\n},\n")
            endif()
        endforeach()
        file(APPEND ${CMAKE_BINARY_DIR}/compile_commands.json "]\n")
    endif()
endfunction()