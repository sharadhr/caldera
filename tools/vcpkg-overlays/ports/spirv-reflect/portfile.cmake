vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/SPIRV-Reflect
    REF "vulkan-sdk-${VERSION}"
    SHA512 79006bca90fdd792cebb9a49bc94070385cb156cf15b2622d2bc4e6a1574ea0deee73db9daeb37f98db143102947c34ca1ea34c99f271d2a97073317158e5376
    HEAD_REF main
    PATCHES
        export-targets-1.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSPIRV_REFLECT_STATIC_LIB=ON
        -DSPIRV_REFLECT_EXAMPLES=OFF
        -DSPIRV_REFLECT_BUILD_TESTS=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-spirv-reflect)

vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/spirv-reflect/spirv_reflect.h" "./include/spirv/unified1/spirv.h" "spirv.h")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

vcpkg_copy_tools(TOOL_NAMES spirv-reflect-pp spirv-reflect AUTO_CLEAN)
