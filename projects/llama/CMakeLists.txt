cmake_minimum_required(VERSION 3.20)
project(llama)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake"
     "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg/scripts/buildsystems")

include(vcpkg)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
# find_package(Qt5 COMPONENTS Widgets CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(
  Qt5
  COMPONENTS Widgets CONFIG
  REQUIRED)
set(CMAKE_AUTOMOC ON)
include(GoogleTest)
include(llama)
include("${CMAKE_CURRENT_LIST_DIR}/modules.cmake")

foreach(MODULE ${MODULE_LIST})
  add_subdirectory(${MODULE})
endforeach()

# rttireg 依赖所有人
foreach(MODULE ${MODULE_LIST})
  if(MODULE STREQUAL foundation)
    llama_link(rtti_registry PUBLIC ${MODULE})
  else()
    llama_link(rtti_registry PRIVATE ${MODULE})
  endif()
endforeach()

llama_docs()
