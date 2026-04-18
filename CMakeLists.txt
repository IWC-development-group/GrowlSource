cmake_minimum_required (VERSION 3.8)
project (growl)

add_library(glfw STATIC IMPORTED)
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glfw")

set_target_properties(
    glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_DIR}/lib/glfw3.lib"
)

set_target_properties(
    glfw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_DIR}/include"
)

add_subdirectory ("thirdparty/imgui")
add_subdirectory ("src")