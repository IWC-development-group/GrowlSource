file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE IMGUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(imgui STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_HEADERS}
)

find_package(OpenGL REQUIRED)
target_link_libraries(imgui PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(imgui PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLFW_DIR}/include
)

target_link_libraries(imgui PRIVATE
    glfw
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)