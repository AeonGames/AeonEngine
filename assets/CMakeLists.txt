include(${CMAKE_SOURCE_DIR}/cmake/base64.cmake)


add_b64_decode_target(decode_aerin
  "${CMAKE_SOURCE_DIR}/assets/aerin/aerin.blend.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Diffuse.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Gloss.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Normal.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Specular.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Opacity.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Normal.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Specular.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Diffuse.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Gloss.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Diffuse.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Normal.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Gloss.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Specular.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Normal.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Diffuse.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Specular.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Gloss.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Diffuse.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Normal.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Gloss.png.b64"
  "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Specular.png.b64"
)

add_custom_target(install_aerin
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_SOURCE_DIR}/game/aerin/textures/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Diffuse.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Gloss.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Normal.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Specular.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Body_Opacity.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Normal.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Specular.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Diffuse.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Gloss.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Diffuse.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Normal.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Gloss.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Shoes_Specular.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Normal.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Diffuse.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Bottom_Specular.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Gloss.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Diffuse.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Normal.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Hair_Gloss.png"
        "${CMAKE_SOURCE_DIR}/assets/aerin/textures/Aerin_Top_Specular.png"
        "${CMAKE_SOURCE_DIR}/game/aerin/textures/"
    DEPENDS decode_aerin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Installing Aerin assets"
)
