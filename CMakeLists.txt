cmake_minimum_required(VERSION 3.16)
project(NetworkSimulator CXX)

# Стандарт C++17 (нужен для std::optional и std::make_shared)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем предупреждения компилятора (хорошая практика)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Все заголовочные файлы находятся в папке include/
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Список всех исходных файлов
set(SOURCES
    src/Packet.cpp
    src/Node.cpp
    src/Host.cpp
    src/Router.cpp
    src/CSMACDMedium.cpp
    src/StatisticsCollector.cpp
    src/TrafficStrategies.cpp
    src/NodeFactory.cpp
    src/SimulationFacade.cpp
    src/main.cpp
)

# Создаём исполняемый файл
add_executable(network_simulator ${SOURCES})

# Инструкции по сборке:
#
#   Windows (PowerShell / Developer Command Prompt):
#     mkdir build && cd build
#     cmake .. -G "NMake Makefiles"   (или -G "Visual Studio 17 2022")
#     cmake --build .
#     .\network_simulator.exe
#
#   Linux / macOS (bash):
#     mkdir build && cd build
#     cmake ..
#     make
#     ./network_simulator
