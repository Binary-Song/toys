cmake_minimum_required(VERSION 3.20)
project(llama)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake"
     "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg/scripts/buildsystems")

include(vcpkg)
include(llama)
include(GoogleTest)

llama_auto_housekeep()

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
# find_package(Qt5 COMPONENTS Widgets CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)

if(gui IN_LIST VCPKG_MANIFEST_FEATURES)
  find_package(OpenGL REQUIRED)
  find_package(glfw3 CONFIG REQUIRED)
  find_package(
    Qt5   CONFIG
    COMPONENTS Widgets 
    REQUIRED)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/src/modules.cmake")

foreach(MODULE ${MODULE_LIST})
  add_subdirectory(src/${MODULE})
endforeach()

llama_docs()
llama_finalize()
