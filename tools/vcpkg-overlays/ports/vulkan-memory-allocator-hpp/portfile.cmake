vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO YaaZ/VulkanMemoryAllocator-Hpp
	REF "v${VERSION}"
	SHA512 6d328c1aebeb6018910986b9bd79e47912a2be87d28dfed43d4a0a5ad747b5b3588a17482e4e2e948a7936e363f778678857aa52212d2959b5994d0b8ddf6a2a
	HEAD_REF master
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_replace_string("${SOURCE_PATH}/include/vk_mem_alloc.hpp" "import VULKAN_HPP_STD_MODULE;" "import std;")

vcpkg_cmake_install()

file(COPY "${SOURCE_PATH}/src/vk_mem_alloc.cppm" DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
