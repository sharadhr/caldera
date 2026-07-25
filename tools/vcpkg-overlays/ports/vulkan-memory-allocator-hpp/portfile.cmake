vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO GrinlexGH/VulkanMemoryAllocator-Hpp
  REF 4a711f97d1d122fe5254ae0a686d17335e595b5c
  SHA512 3deaebbb0d538dbbffbb4162541e20f8230ef640695a557482259337a34e68407594cb4c056bde4997e84a6087bdab7f624a68198ea59475375ce9d328b92f47
  HEAD_REF master
  PATCHES
    001-std-expected-converters.diff
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}/include"
	OPTIONS
		-DVMA_HPP_ENABLE_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
