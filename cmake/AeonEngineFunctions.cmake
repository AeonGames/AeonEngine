# Copyright (C) 2017 AeonGames, Rodrigo Jose Hernandez Cordoba
# Licensed under the terms of the Apache 2.0 License.

function(generate_compile_commands target sources)
    if(MSVC AND CMAKE_EXPORT_COMPILE_COMMANDS)
        get_target_property(INCLUDE_DIRECTORIES ${target} INCLUDE_DIRECTORIES)
        foreach(INCLUDE_DIRECTORY ${INCLUDE_DIRECTORIES})
            set(INCLUDE_FLAGS "${INCLUDE_FLAGS} -I${INCLUDE_DIRECTORY}")
        endforeach()
        get_target_property(COMPILE_FLAGS ${target} COMPILE_FLAGS)
        get_target_property(COMPILE_OPTIONS ${target} COMPILE_OPTIONS)
        file(WRITE ${CMAKE_BINARY_DIR}/compile_commands.json "[\n")
        foreach(SOURCE_FILE ${sources})
            get_source_file_property(LANGUAGE ${SOURCE_FILE} LANGUAGE)
            get_source_file_property(LOCATION ${SOURCE_FILE} LOCATION)
            get_source_file_property(OBJECT_OUTPUTS ${SOURCE_FILE} OBJECT_OUTPUTS)
            
            string(REPLACE "/" "-" ${LANGUAGE}_FLAGS ${CMAKE_${LANGUAGE}_FLAGS})
            
            if(LANGUAGE STREQUAL "CXX" OR LANGUAGE STREQUAL "C")
                file(APPEND ${CMAKE_BINARY_DIR}/compile_commands.json "{\n  \"directory\": \"${CMAKE_CURRENT_SOURCE_DIR}\",\n  \"command\": \"clang-cl.exe ${INCLUDE_FLAGS} ${${LANGUAGE}_FLAGS} ${COMPILE_FLAGS} -c ${LOCATION}\",\n  \"file\": \"${LOCATION}\"\n},\n")
            endif()
        endforeach()
        file(APPEND ${CMAKE_BINARY_DIR}/compile_commands.json "]\n")
    endif()
endfunction()
