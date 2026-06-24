add_subdirectory(lib/bbui)

cmake_minimum_required(VERSION 3.27)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

project(typesetter)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(ariaflow ${SRC_FILES})
add_dependencies(ariaflow bbui)
if(WIN32)
    target_link_libraries(ariaflow PRIVATE ${CMAKE_SOURCE_DIR}/lib/bbui/lib/GLFW/lib-vc2019/glfw3.lib)
    target_link_libraries(ariaflow PRIVATE ${CMAKE_SOURCE_DIR}/lib/bbui/bin/bbui.lib)
else()
    target_link_libraries(ariaflow PRIVATE glfw pthread)
    target_link_libraries(ariaflow PRIVATE bbui)
endif()
target_include_directories(ariaflow PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(ariaflow PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_include_directories(ariaflow PRIVATE ${CMAKE_SOURCE_DIR}/lib/bbui/inc)

set_property(GLOBAL PROPERTY LINKER_LANGUAGE CXX)
set_property(TARGET ariaflow PROPERTY CXX_STANDARD 20)
if(WIN32)
    set(CMAKE_C_COMPILER "cl.exe")
    set(CMAKE_CXX_COMPILER "cl.exe")

    add_compile_options("/ZI")
    add_link_options("/DEBUG")
else()
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

    add_compile_options("-g")
    add_link_options("-g")
    add_compile_options("-O3")
    add_compile_options(
        -fno-delete-null-pointer-checks
        -Wno-tautological-undefined-compare
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-missing-field-initializers
    )
endif()
