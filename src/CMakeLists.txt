cmake_minimum_required(VERSION 3.20)
project(growl)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_GENERATOR_PLATFORM x64)

# file(GLOB_RECURSE SOURCES "*.cpp")
# file(GLOB_RECURSE HEADERS "*.h")

set(SOURCES
    growl_main.cpp
    application.cpp
    layer_stack.cpp
    instances.cpp
    session.cpp
 "event_worker.cpp" "string_converting.cpp")

add_executable(growl ${SOURCES}) # !!!
set_target_properties(growl PROPERTIES CXX_STANDARD 23)

set(LIBSHOUT_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/libshout")
set(RESOURCES_ROOT "${CMAKE_SOURCE_DIR}/res")

target_include_directories(growl PRIVATE "${LIBSHOUT_ROOT}/include")

target_link_libraries(growl PRIVATE
    "${LIBSHOUT_ROOT}/lib/libshout-3.lib"
    ws2_32
    winmm
)

find_package(OpenGL REQUIRED)
target_link_libraries(growl PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(growl PRIVATE ${OPENGL_LIBRARIES})

target_link_libraries(growl PRIVATE
    glfw
    imgui
)

add_custom_command(TARGET growl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LIBSHOUT_ROOT}/bin/"
        $<TARGET_FILE_DIR:growl>
)

add_custom_command(TARGET growl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCES_ROOT}"
        $<TARGET_FILE_DIR:growl>
)