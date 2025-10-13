vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/Vulkan-Headers
    REF "v${VERSION}"
    SHA512 306f2d1922441240f28cafcb27f9ff97c363cc464dcd3d02a94729c0b8b1e17e4082f22e379b14c9db12a35be221a86369077bcec5b59a310caa01ae4ce378d3
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release) # header-only port

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DVULKAN_HEADERS_ENABLE_TESTS=OFF
)
vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
