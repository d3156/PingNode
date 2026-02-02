cmake_minimum_required(VERSION 3.16)
project(PingNodePlugin VERSION 1.0.3 LANGUAGES CXX)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/workspace.cmake")
create_target(plugin)

find_package(Boost 1.8 REQUIRED COMPONENTS json thread system)
target_link_libraries(PingNodePlugin PRIVATE Boost::system Boost::thread Boost::json)

dependency("../PingNodeModel" PingNodeModel)
dependency("../../MetricsModel" MetricsModel)
