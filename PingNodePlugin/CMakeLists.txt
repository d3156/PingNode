cmake_minimum_required(VERSION 3.16)
project(PingNodePlugin
  VERSION 1.0.0
  LANGUAGES CXX
)
add_compile_definitions(PingNodePlugin_VERSION="${PROJECT_VERSION}" )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(PingNodePlugin SHARED ${SRC_FILES})

target_include_directories(PingNodePlugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Put resulting .so into workspace/Plugins
set_target_properties(PingNodePlugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../Plugins"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../Plugins"
  OUTPUT_NAME "PingNodePlugin"
)

find_package(PluginCore REQUIRED)
target_link_libraries(PingNodePlugin PRIVATE d3156::PluginCore)


if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel/CMakeLists.txt")
    message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel not found, downloading...")
    
    include(FetchContent)
    FetchContent_Declare(
        ${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel
        GIT_REPOSITORY https://github.com/d3156/${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel)
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../PingNodeModel" 
                     "${CMAKE_BINARY_DIR}/_deps/PingNodeModel")
endif()

target_link_libraries(PingNodePlugin PRIVATE PingNodeModel)

