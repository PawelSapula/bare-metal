cmake_minimum_required(VERSION 3.16)
project(vulkan)

# Source - https://stackoverflow.com/a/45781334
# Posted by Jörn Reimerdes
# Retrieved 2026-09-20, License - CC BY-SA 3.0

# cmake -S . -B build \ -- Initialization, remember to source the setup_env.sh in the SDK.
# -DCMAKE_PREFIX_PATH="$VULKAN_SDK"

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

# Early Apple Silicon (Supposedly M1-M3)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/")
include_directories(include)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.c


                              modules/buffers.c
                              modules/devices.c
                              modules/pipeline.c
                              modules/swapchain.c
                              modules/draw.c
                              modules/vk_instance.c

                              include/buffers.h
                              include/devices.h
                              include/pipeline.h
                              include/swapchain.h
                              include/draw.h
                              include/vk_instance.h

                              modules/vk_utils.c
                              modules/vk_swapchain_utils.c
                              modules/vk_shader_utils.c

                              include/vk_data.h
                              include/vk_utils.h
                              include/vk_swapchain_utils.h
                              include/vk_shader_utils.h
                              include/utils.h

                              mystd/mystd.h
                              mystd/mystd_conf.h
                              mystd/dyn_arr.c
                              mystd/dyn_arr.h
                            )


target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)

