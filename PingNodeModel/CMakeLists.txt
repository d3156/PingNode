cmake_minimum_required(VERSION 3.16)
project(PingNodeModel 
  VERSION 1.0.0
  LANGUAGES CXX
)

add_compile_definitions(PingNodeModel_VERSION="${PROJECT_VERSION}" )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

add_library(PingNodeModel STATIC  ${SRC_FILES})

target_include_directories(PingNodeModel
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
