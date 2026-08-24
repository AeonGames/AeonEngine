include(${CMAKE_SOURCE_DIR}/cmake/base64.cmake)

add_blender_asset(aerin
    BLEND assets/aerin/aerin.blend
    DESTINATION game/aerin
    RESOURCE_PREFIX aerin
)

# The backdrop carries its textures packed inside the .blend; the exporter
# writes them out into game/backdrop/textures.
add_blender_asset(backdrop
    BLEND assets/backdrop/backdrop.blend
    DESTINATION game/backdrop
    RESOURCE_PREFIX backdrop
)

add_blender_asset(polesign
    BLEND assets/polesign/polesign.blend
    DESTINATION game/polesign
    RESOURCE_PREFIX polesign
    TEXTURES "assets/polesign/textures/polesign.png"
)

add_blender_asset(sponza
    MODE SCENE
    BLEND assets/sponza/sponza.blend
    DESTINATION game/sponza
    RESOURCE_PREFIX sponza
)

add_subdirectory(shadercode)
