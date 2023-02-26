cmake_minimum_required(VERSION 3.10)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/conan)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/conan)

set(CMAKE_CXX_STANDARD {{ cpp_std }})
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
    {{ project_name }}
    VERSION {{ project_version }}
    DESCRIPTION "{{ project_description }}"
    LANGUAGES CXX
)

set(PROJECT_AUTHOR "{{ author_name }}")
set(PROJECT_AUTHOR_GITHUB "{{ author_github }}")

add_subdirectory(configured_files)

find_package(fmt CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME} 
        main.cpp
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE 
        fmt::fmt
)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/configured_files/include
)
