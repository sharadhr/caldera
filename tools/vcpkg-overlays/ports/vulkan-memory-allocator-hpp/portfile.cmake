vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO YaaZ/VulkanMemoryAllocator-Hpp
	REF 579c834c5221cd16c66a2f3fa1cee253714aae82
	SHA512 de3f97fddc28926cf26101c4976fc8368e5313cd09be5ea8063a694e8b0bb8af8620419853af19746cd308e7e2ad4fe42ae585656a2233f7a0b07061611ac0dd
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}/include"
	OPTIONS
		-DVMA_HPP_ENABLE_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
