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
#    "clustered_phong_skinned|vert=skinned_mesh.vert|frag=clustered_phong.frag"
    "clustered_phong|vert=static_mesh.vert|frag=clustered_phong.frag"
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

# Packed binary (.pln) forms of a subset of the generated pipelines. The engine
# loads the .txt at runtime; the .pln are produced for tooling/parity only.
#
# Each .pln conversion is attached to the same shader-pipelines target as the
# .txt it reads (the target is declared below, after this loop). Keeping the
# packing and the conversion in one target is what makes a parallel clean build
# correct: the Make generator then emits both rules in a single build.make, where
# each .pln depends on its .txt as a plain file prerequisite, so the packing
# always runs before the matching conversion at any -j. When the .pln lived in a
# separate target the .txt became a cross-target dependency, ordered only at the
# target level; a high-parallelism clean build could outrun that ordering and try
# to convert a not-yet-packed pipeline ("File .../<name>.txt Not Found").
set(PIPELINE_BINARY_NAMES
    simple_phong plain_red noop_compute cluster_build
    light_cull cluster_mark lighting skinning)
set(PIPELINE_BINARIES)
foreach(name IN LISTS PIPELINE_BINARY_NAMES)
    add_custom_command(
        OUTPUT  ${PIPELINE_OUTPUT_DIR}/${name}.pln
        COMMAND $<TARGET_FILE:aeontool> convert
                -i ${PIPELINE_OUTPUT_DIR}/${name}.txt
                -o ${PIPELINE_OUTPUT_DIR}/${name}.pln
        DEPENDS ${PIPELINE_OUTPUT_DIR}/${name}.txt aeontool ProtoBufClasses
        COMMENT "Converting game/shaders/${name}.txt"
        VERBATIM
    )
    list(APPEND PIPELINE_BINARIES ${PIPELINE_OUTPUT_DIR}/${name}.pln)
endforeach()

# One target owns every generated pipeline artifact: all the .txt (including the
# runtime-only ones with no .pln) and the .pln binaries. Building it on a fresh
# checkout brings the whole game/shaders/ tree up to date.
add_custom_target(shader-pipelines
    DEPENDS ${GENERATED_PIPELINES} ${PIPELINE_BINARIES}
    SOURCES ${GENERATED_PIPELINES}
    COMMENT "Generating shader pipeline .txt and .pln files"
)
