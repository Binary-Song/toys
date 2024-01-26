cmake_minimum_required(VERSION 3.20)
project(llama)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg/scripts/buildsystems")

include(vcpkg)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Widgets CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

enable_testing()
include(GoogleTest)

include(llama)

add_subdirectory(foundation)

llama_docs()