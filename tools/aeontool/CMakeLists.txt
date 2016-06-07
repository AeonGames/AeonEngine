set(AEONTOOL_HEADERS)
set(AEONTOOL_SOURCES Main.cpp)

include_directories(
                    ${PROTOBUF_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}
)

PROTOBUF_GENERATE_CPP(PROTOBUF_SOURCES PROTOBUF_HEADERS
    ${CMAKE_SOURCE_DIR}/proto/Configuration.proto
    ${CMAKE_SOURCE_DIR}/proto/vector2.proto    
    ${CMAKE_SOURCE_DIR}/proto/vector3.proto
    ${CMAKE_SOURCE_DIR}/proto/vector4.proto
    ${CMAKE_SOURCE_DIR}/proto/mesh.proto
    ${CMAKE_SOURCE_DIR}/proto/shader_program.proto
)

source_group("protobuf" FILES ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_executable(aeontool ${AEONTOOL_HEADERS} ${AEONTOOL_SOURCES} ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})
target_link_libraries(aeontool ${PROTOBUF_LIBRARIES})
if(MSVC)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-WX -DPROTOBUF_USE_DLLS -D_CRT_SECURE_NO_WARNINGS")
elseif(MINGW)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-Wall -DPROTOBUF_USE_DLLS")
elseif(UNIX)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-Wall")
endif()

