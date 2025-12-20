vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO YaaZ/VulkanMemoryAllocator-Hpp
	REF "v${VERSION}"
	SHA512 72fccbba9ad422baa0f9e9389a72ccf4aa760ea1f15ecdf6d08604d60c25969938a300db6350363841ba66a40ca7804265477faeb601e142de9d7211da08ada2
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}/include"
	OPTIONS
		-DVMA_HPP_ENABLE_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
