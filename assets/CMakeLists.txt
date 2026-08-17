include(${CMAKE_SOURCE_DIR}/cmake/base64.cmake)

add_blender_asset(aerin
    BLEND assets/aerin/aerin.blend
    DESTINATION game/aerin
    RESOURCE_PREFIX aerin
    TEXTURES
        "assets/aerin/textures/Aerin_Body_Diffuse.png"
        "assets/aerin/textures/Aerin_Body_Gloss.png"
        "assets/aerin/textures/Aerin_Body_Normal.png"
        "assets/aerin/textures/Aerin_Body_Specular.png"
        "assets/aerin/textures/Aerin_Body_Opacity.png"
        "assets/aerin/textures/Aerin_Hair_Normal.png"
        "assets/aerin/textures/Aerin_Hair_Specular.png"
        "assets/aerin/textures/Aerin_Shoes_Diffuse.png"
        "assets/aerin/textures/Aerin_Shoes_Gloss.png"
        "assets/aerin/textures/Aerin_Bottom_Diffuse.png"
        "assets/aerin/textures/Aerin_Shoes_Normal.png"
        "assets/aerin/textures/Aerin_Bottom_Gloss.png"
        "assets/aerin/textures/Aerin_Shoes_Specular.png"
        "assets/aerin/textures/Aerin_Bottom_Normal.png"
        "assets/aerin/textures/Aerin_Top_Diffuse.png"
        "assets/aerin/textures/Aerin_Bottom_Specular.png"
        "assets/aerin/textures/Aerin_Top_Gloss.png"
        "assets/aerin/textures/Aerin_Hair_Diffuse.png"
        "assets/aerin/textures/Aerin_Top_Normal.png"
        "assets/aerin/textures/Aerin_Hair_Gloss.png"
        "assets/aerin/textures/Aerin_Top_Specular.png"
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
    TEXTURES
        "assets/sponza/textures/arch_stone_wall_01_BaseColor.png"
        "assets/sponza/textures/arch_stone_wall_01_Metalness.png"
        "assets/sponza/textures/arch_stone_wall_01_Normal.png"
        "assets/sponza/textures/arch_stone_wall_01_Roughness.png"
        "assets/sponza/textures/brickwall_01_BaseColor.png"
        "assets/sponza/textures/brickwall_01_Metalness.png"
        "assets/sponza/textures/brickwall_01_Normal.png"
        "assets/sponza/textures/brickwall_01_Roughness.png"
        "assets/sponza/textures/brickwall_02_BaseColor.png"
        "assets/sponza/textures/brickwall_02_Metalness.png"
        "assets/sponza/textures/brickwall_02_Normal.png"
        "assets/sponza/textures/brickwall_02_Roughness.png"
        "assets/sponza/textures/ceiling_plaster_01_BaseColor.png"
        "assets/sponza/textures/ceiling_plaster_01_Metalness.png"
        "assets/sponza/textures/ceiling_plaster_01_Normal.png"
        "assets/sponza/textures/ceiling_plaster_01_Roughness.png"
        "assets/sponza/textures/ceiling_plaster_02_BaseColor.png"
        "assets/sponza/textures/ceiling_plaster_02_Roughness.png"
        "assets/sponza/textures/col_1stfloor_BaseColor.png"
        "assets/sponza/textures/col_1stfloor_Metalness.png"
        "assets/sponza/textures/col_1stfloor_Normal.png"
        "assets/sponza/textures/col_1stfloor_Roughness.png"
        "assets/sponza/textures/col_brickwall_01_BaseColor.png"
        "assets/sponza/textures/col_brickwall_01_Metalness.png"
        "assets/sponza/textures/col_brickwall_01_Normal.png"
        "assets/sponza/textures/col_brickwall_01_Roughness.png"
        "assets/sponza/textures/col_head_1stfloor_BaseColor.png"
        "assets/sponza/textures/col_head_1stfloor_Metalness.png"
        "assets/sponza/textures/col_head_1stfloor_Normal.png"
        "assets/sponza/textures/col_head_1stfloor_Roughness.png"
        "assets/sponza/textures/col_head_2ndfloor_02_BaseColor.png"
        "assets/sponza/textures/col_head_2ndfloor_02_Metalness.png"
        "assets/sponza/textures/col_head_2ndfloor_02_Normal.png"
        "assets/sponza/textures/col_head_2ndfloor_02_Roughness.png"
        "assets/sponza/textures/col_head_2ndfloor_03_BaseColor.png"
        "assets/sponza/textures/col_head_2ndfloor_03_Metalness.png"
        "assets/sponza/textures/col_head_2ndfloor_03_Normal.png"
        "assets/sponza/textures/col_head_2ndfloor_03_Roughness.png"
        "assets/sponza/textures/dirt_decal_01.png"
        "assets/sponza/textures/dirt_decal_01_mask_gltf.png"
        "assets/sponza/textures/door_stoneframe_01_BaseColor.png"
        "assets/sponza/textures/door_stoneframe_01_Metalness.png"
        "assets/sponza/textures/door_stoneframe_01_Normal.png"
        "assets/sponza/textures/door_stoneframe_01_Roughness.png"
        "assets/sponza/textures/door_stoneframe_02_BaseColor.png"
        "assets/sponza/textures/door_stoneframe_02_Metalness.png"
        "assets/sponza/textures/door_stoneframe_02_Normal.png"
        "assets/sponza/textures/door_stoneframe_02_Roughness.png"
        "assets/sponza/textures/floor_tiles_01_BaseColor.png"
        "assets/sponza/textures/floor_tiles_01_Metalness.png"
        "assets/sponza/textures/floor_tiles_01_Normal.png"
        "assets/sponza/textures/floor_tiles_01_Roughness.png"
        "assets/sponza/textures/lionhead_01_BaseColor.png"
        "assets/sponza/textures/lionhead_01_Metalness.png"
        "assets/sponza/textures/lionhead_01_Normal.png"
        "assets/sponza/textures/lionhead_01_Roughness.png"
        "assets/sponza/textures/metal_door_01_BaseColor.png"
        "assets/sponza/textures/metal_door_01_Metalness.png"
        "assets/sponza/textures/metal_door_01_Normal.png"
        "assets/sponza/textures/metal_door_01_Roughness.png"
        "assets/sponza/textures/ornament_01_BaseColor.png"
        "assets/sponza/textures/ornament_01_Metalness.png"
        "assets/sponza/textures/ornament_01_Normal.png"
        "assets/sponza/textures/ornament_01_Roughness.png"
        "assets/sponza/textures/roof_tiles_01_BaseColor.png"
        "assets/sponza/textures/roof_tiles_01_Metalness.png"
        "assets/sponza/textures/roof_tiles_01_Normal.png"
        "assets/sponza/textures/roof_tiles_01_Roughness.png"
        "assets/sponza/textures/stones_2ndfloor_01_BaseColor.png"
        "assets/sponza/textures/stones_2ndfloor_01_Metalness.png"
        "assets/sponza/textures/stones_2ndfloor_01_Normal.png"
        "assets/sponza/textures/stones_2ndfloor_01_Roughness.png"
        "assets/sponza/textures/stone_01_tile_BaseColor.png"
        "assets/sponza/textures/stone_01_tile_Metalness.png"
        "assets/sponza/textures/stone_01_tile_Normal.png"
        "assets/sponza/textures/stone_01_tile_Roughness.png"
        "assets/sponza/textures/stone_trims_01_BaseColor.png"
        "assets/sponza/textures/stone_trims_01_Metalness.png"
        "assets/sponza/textures/stone_trims_01_Normal.png"
        "assets/sponza/textures/stone_trims_01_Roughness.png"
        "assets/sponza/textures/stone_trims_02_BaseColor.png"
        "assets/sponza/textures/stone_trims_02_Metalness.png"
        "assets/sponza/textures/stone_trims_02_Normal.png"
        "assets/sponza/textures/stone_trims_02_Roughness.png"
        "assets/sponza/textures/window_frame_01_BaseColor.png"
        "assets/sponza/textures/window_frame_01_Metalness.png"
        "assets/sponza/textures/window_frame_01_Normal.png"
        "assets/sponza/textures/window_frame_01_Roughness.png"
        "assets/sponza/textures/wood_door_01_BaseColor.png"
        "assets/sponza/textures/wood_door_01_Metalness.png"
        "assets/sponza/textures/wood_door_01_Normal.png"
        "assets/sponza/textures/wood_door_01_Roughness.png"
        "assets/sponza/textures/wood_tile_01_BaseColor.png"
        "assets/sponza/textures/wood_tile_01_Metalness.png"
        "assets/sponza/textures/wood_tile_01_Normal.png"
        "assets/sponza/textures/wood_tile_01_Roughness.png"
        "assets/sponza/textures/kloppenheim_05_4k.hdr"
)

add_subdirectory(shadercode)
