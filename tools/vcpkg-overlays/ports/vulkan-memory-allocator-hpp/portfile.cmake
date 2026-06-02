vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO YaaZ/VulkanMemoryAllocator-Hpp
	REF "v${VERSION}"
	SHA512 14d853962f7410a6495c9a6c0cd7ce8b977e7eca5f4e243c9e442461d25d3e7d7671097e3962a7560af07a9618069eb01072e69a52220e61c21dc0deb52b9b75
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}/include"
	OPTIONS
		-DVMA_HPP_ENABLE_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
