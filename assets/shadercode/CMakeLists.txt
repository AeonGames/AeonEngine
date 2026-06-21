# Generates the .txt pipeline files under game/shaders/ from the shader
# source files (.vert/.frag/...) checked in under this directory.
#
# Each pipeline is declared with explicit per-stage source files so that
# several pipelines can share a single fragment (or vertex) shader without
# having to duplicate the file on disk.
#
# Entry format: "<pipeline-name>|<stage>=<file>|<stage>=<file>|..."
# Supported stages: vert, frag, comp, tesc, tese, geom.
# A field may also be "topology=<CLASS>" to set the pipeline topology class
# (a PipelineMsg::TopologyClass enum name, e.g. TRIANGLE or LINE).
#
# To regenerate the pipeline .txt files, build the `shader-pipelines`
# target (e.g. `make shader-pipelines`).

set(PIPELINES
#    "diffuse_map_phong|vert=diffuse_map_phong.vert|frag=diffuse_map_phong.frag"
    "diffuse_map_phong_no_skeleton|vert=diffuse_map_phong_no_skeleton.vert|frag=diffuse_map_phong.frag"
    "cluster_build|comp=cluster_build.comp"
    "light_cull|comp=light_cull.comp"
    "cluster_mark|vert=cluster_mark.vert|frag=cluster_mark.frag"
    "lighting|comp=lighting.0.comp|comp=lighting.1.comp"
    "skinning|comp=skinning.comp"
    "shadow_depth|vert=shadow_depth.vert|frag=shadow_depth.frag"
    "point_shadow_depth|vert=point_shadow_depth.vert|geom=point_shadow_depth.geom|frag=point_shadow_depth.frag|Vulkan:vert=point_shadow_depth_mv.vert|Vulkan:geom"
    # Engine debug + example pipelines (sources recovered from the previously
    # hand-committed game/shaders/*.txt, now generated like every other pipeline).
    "debug_grid|vert=debug_grid.vert|frag=debug_grid.frag|topology=TRIANGLE"
    "solid_color|vert=solid_color.vert|frag=solid_color.frag|topology=LINE"
    "simple_phong|vert=simple_phong.vert|frag=simple_phong.frag"
    "plain_red|vert=plain_red.vert|frag=plain_red.frag"
    "noop_compute|comp=noop_compute.comp"
)

set(SHADER_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(PIPELINE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/game/shaders)

set(GENERATED_PIPELINES)
foreach(entry IN LISTS PIPELINES)
    string(REPLACE "|" ";" parts "${entry}")
    list(GET parts 0 name)
    list(LENGTH parts parts_len)

    set(stage_args)
    set(stage_deps)
    math(EXPR last_index "${parts_len} - 1")
    foreach(i RANGE 1 ${last_index})
        list(GET parts ${i} pair)
        # Each field is one of:
        #   stage=file            default variant (all renderers)
        #   Set:stage=file        variant for the renderer set Set (comma list)
        #   Set:stage             disable the stage for the renderer set Set
        #   topology=CLASS        pipeline topology class (e.g. TRIANGLE, LINE)
        # Set uses ',' to list renderers ('|' is the entry field separator).
        if(pair MATCHES "^topology=(.+)$")
            list(APPEND stage_args --topology ${CMAKE_MATCH_1})
        elseif(pair MATCHES "^([^=]+)=(.+)$")
            set(lhs "${CMAKE_MATCH_1}")
            set(file_path "${SHADER_SOURCE_DIR}/${CMAKE_MATCH_2}")
            if(lhs MATCHES "^([^:]+):([A-Za-z]+)$")
                list(APPEND stage_args --variant ${CMAKE_MATCH_2} ${CMAKE_MATCH_1} ${file_path})
            else()
                list(APPEND stage_args --${lhs} ${file_path})
            endif()
            list(APPEND stage_deps ${file_path})
        elseif(pair MATCHES "^([^:]+):([A-Za-z]+)$")
            list(APPEND stage_args --disable ${CMAKE_MATCH_2} ${CMAKE_MATCH_1})
        else()
            message(FATAL_ERROR "Invalid pipeline stage spec '${pair}' in pipeline '${name}'")
        endif()
    endforeach()

    set(out_file ${PIPELINE_OUTPUT_DIR}/${name}.txt)
    add_custom_command(
        OUTPUT  ${out_file}
        COMMAND $<TARGET_FILE:aeontool> pipeline
                ${stage_args}
                -o ${out_file}
        DEPENDS aeontool ${stage_deps}
        COMMENT "Packing pipeline ${name}.txt"
        VERBATIM
    )
    list(APPEND GENERATED_PIPELINES ${out_file})
endforeach()

add_custom_target(shader-pipelines
    DEPENDS ${GENERATED_PIPELINES}
    SOURCES ${GENERATED_PIPELINES}
    COMMENT "Generating shader pipeline .txt files"
)
