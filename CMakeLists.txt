cmake_minimum_required(VERSION 3.10)
project(NetworkSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(network_simulator
    src/Packet.cpp
    src/Node.cpp
    src/Host.cpp
    src/Router.cpp
    src/CSMACDMedium.cpp
    src/StatisticsCollector.cpp
    src/TrafficStrategies.cpp
    src/PoissonStrategy.cpp
    src/NetworkVisualizer.cpp
    src/NodeFactory.cpp
    src/SimulationFacade.cpp
    src/main.cpp
)

target_include_directories(network_simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT DEFINED INSTALL_GTEST)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
