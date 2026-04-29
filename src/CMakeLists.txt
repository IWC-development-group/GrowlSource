cmake_minimum_required(VERSION 3.20)
project(growl)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_GENERATOR_PLATFORM x64)

set(SOURCES
    growl_main.cpp
    application.cpp
    layer_stack.cpp
    instances.cpp
    session.cpp
 "event_worker.cpp" "string_converting.cpp" "growl.cpp" "ui_layer.cpp")

add_executable(growl ${SOURCES}) # !!!
set_target_properties(growl PROPERTIES CXX_STANDARD 23)

set(LIBSHOUT_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/libshout")
set(TAGLIB_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/taglib")
set(RESOURCES_ROOT "${CMAKE_SOURCE_DIR}/res")

target_include_directories(growl PRIVATE
    "${LIBSHOUT_ROOT}/include"
    "${TAGLIB_ROOT}/include"
)

target_link_directories(growl PRIVATE
    "${LIBSHOUT_ROOT}/lib"
    "${TAGLIB_ROOT}/lib/"
)

find_package(OpenGL REQUIRED)
target_link_libraries(growl PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(growl PRIVATE ${OPENGL_LIBRARIES})

target_link_libraries(growl PRIVATE
    libshout-3
    ws2_32
    winmm
    glfw
    imgui
    tag
)

add_custom_command(TARGET growl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LIBSHOUT_ROOT}/bin/"
        $<TARGET_FILE_DIR:growl>
)

add_custom_command(TARGET growl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TAGLIB_ROOT}/bin/"
        $<TARGET_FILE_DIR:growl>
)

add_custom_command(TARGET growl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCES_ROOT}"
        $<TARGET_FILE_DIR:growl>
)