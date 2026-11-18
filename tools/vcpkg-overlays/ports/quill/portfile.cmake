vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO odygrd/quill
    # Unreleased master commit with the fix for odygrd/quill#1012 (import std on MSVC).
    # Return to "v${VERSION}" once v14.0.0 is released.
    REF 8ecfde27bddbbd410103df97e8b098bf1a1bffb6
    SHA512 71e488cb8fd57453c6c379d28669d4e4f7b3d2d061698500f7058dee9ced9029362bfaf942d0c0c6df203cb7cef977438203afb0a50191f93341fe84e3fdac4e
    HEAD_REF master
    PATCHES
        0001-install-quill-module.patch
        0002-module-without-cxx-standard.patch
)

if(VCPKG_TARGET_IS_ANDROID)
    set(ADDITIONAL_OPTIONS -DQUILL_NO_THREAD_NAME_SUPPORT=ON)
endif()

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
OPTIONS
    ${ADDITIONAL_OPTIONS}
    -DQUILL_BUILD_MODULE=ON
    -DQUILL_IMPORT_STD=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/quill)

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
