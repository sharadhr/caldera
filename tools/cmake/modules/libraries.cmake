include_guard(GLOBAL)

set(SDL2_NO_MWINDOWS ON)

find_package(argparse CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(quill CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include(vulkan-cxx-modules)
