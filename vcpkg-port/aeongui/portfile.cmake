vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/AeonGames/AeonGUI.git
    REF bde5b8c729d76bc8dbdcb9b9b31e261009fd8982
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DAEONGUI_BACKEND=Cairo
        -DBUILD_UNIT_TESTS=OFF
        -DENABLE_HTML=ON
        -DDISABLE_DEMOS=ON
        -DFETCHCONTENT_FULLY_DISCONNECTED=OFF
)

vcpkg_cmake_build()
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME aeongui CONFIG_PATH share/aeongui)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
