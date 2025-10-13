
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/SPIRV-Headers
    REF "vulkan-sdk-${VERSION}"
    SHA512 470bcffffd10662ea39282a3546cf2d963dc8344bde93bf0a14d4390cdca96ab8d77351944775cb05d4affc8d2333ee161e9265eedb60f3e548c3eeb7defb051
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH "share/cmake/SPIRV-Headers")
vcpkg_fixup_pkgconfig()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
