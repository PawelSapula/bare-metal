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
add_library(ImGui STATIC IMPORTED) # External library

set_target_properties(ImGui PROPERTIES # Specify location & Dependent files
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libImGui.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)


file(GLOB_RECURSE MODULES CONFIGURE_DEPENDS modules/*.c) #CONFIGURE_DEPENDS - Rebuilts project when new files
file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS include/*.h)
file(GLOB_RECURSE LIB lib/*.h)
file(GLOB_RECURSE MYSTD_LIB mystd/*.h)

add_executable(${PROJECT_NAME} 
  main.c
  ${MODULES}
  ${INCLUDES}
  ${LIB}
  ${MYSTD_LIB}
)


target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} ImGui
)

set_property(TARGET ${PROJECT_NAME} # Use Cpp linker due to imgui
  PROPERTY LINKER_LANGUAGE CXX
)
