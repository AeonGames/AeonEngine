# Copyright (C) 2016-2023,2025 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

# Blender Addon Packaging Targets
set(BLENDER_ADDONS_DIR "${CMAKE_SOURCE_DIR}/tools/blender/addons")
set(BLENDER_ADDONS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/blender_addons")
set(BLENDER_ADDONS_STAGING_DIR "${BLENDER_ADDONS_OUTPUT_DIR}/staging")

# Create output and staging directories
file(MAKE_DIRECTORY "${BLENDER_ADDONS_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${BLENDER_ADDONS_STAGING_DIR}")

# List of all Blender addons
set(BLENDER_ADDONS
    armature_lossless_connect
    io_animation_anm
    io_collision_cln
    io_images
    io_mesh_msh
    io_model_mdl
    io_skeleton_skl
    io_scene_unreal
    obj_duplicate_mesh_detector
)

# Create individual zip targets for each addon
foreach(ADDON ${BLENDER_ADDONS})
    set(ADDON_DIR "${BLENDER_ADDONS_DIR}/${ADDON}")
    set(ADDON_STAGING_DIR "${BLENDER_ADDONS_OUTPUT_DIR}/staging/${ADDON}")
    set(ZIP_FILE "${BLENDER_ADDONS_OUTPUT_DIR}/${ADDON}.zip")

    # Gather all files in the addon directory (excluding __pycache__ and other temp files)
    file(GLOB_RECURSE ADDON_FILES
         "${ADDON_DIR}/*.py"
         "${ADDON_DIR}/*.toml"
         "${ADDON_DIR}/*.txt"
         "${ADDON_DIR}/*.md")

    # Filter out __pycache__ files from the list
    list(FILTER ADDON_FILES EXCLUDE REGEX "/__pycache__/")

    # Create custom target to zip the addon
    add_custom_command(
        OUTPUT "${ZIP_FILE}"
        # Clean the specific addon staging directory
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${ADDON_STAGING_DIR}"
        # Copy addon to staging
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ADDON_DIR}" "${ADDON_STAGING_DIR}"
        # Remove __pycache__ from staging
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${ADDON_STAGING_DIR}/__pycache__"
        # Create the zip from staging
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${ZIP_FILE}" --format=zip "${ADDON}"
        WORKING_DIRECTORY "${BLENDER_ADDONS_STAGING_DIR}"
        DEPENDS ${ADDON_FILES}
        COMMENT "Packaging ${ADDON} addon into ${ADDON}.zip"
    )

    # Create a target for this specific addon
    add_custom_target(package_${ADDON}
        DEPENDS "${ZIP_FILE}"
        COMMENT "Package ${ADDON} addon"
    )

    # Add to the list for the parent target
    list(APPEND ADDON_ZIP_TARGETS package_${ADDON})
endforeach()

# Create parent target that packages all addons
add_custom_target(package_blender_addons
    DEPENDS ${ADDON_ZIP_TARGETS}
    COMMENT "Packaging all Blender addons"
)

# Optional: Add a message to show where the zip files are created
add_custom_command(
    TARGET package_blender_addons POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Blender addon packages created in: ${BLENDER_ADDONS_OUTPUT_DIR}"
)
