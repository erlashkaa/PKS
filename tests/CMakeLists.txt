set(SIM_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Packet.cpp
    ${CMAKE_SOURCE_DIR}/src/Node.cpp
    ${CMAKE_SOURCE_DIR}/src/Host.cpp
    ${CMAKE_SOURCE_DIR}/src/Router.cpp
    ${CMAKE_SOURCE_DIR}/src/CSMACDMedium.cpp
    ${CMAKE_SOURCE_DIR}/src/StatisticsCollector.cpp
    ${CMAKE_SOURCE_DIR}/src/TrafficStrategies.cpp
    ${CMAKE_SOURCE_DIR}/src/PoissonStrategy.cpp
    ${CMAKE_SOURCE_DIR}/src/NetworkVisualizer.cpp
    ${CMAKE_SOURCE_DIR}/src/NodeFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/SimulationFacade.cpp
)
add_library(sim_core OBJECT ${SIM_SOURCES})
target_include_directories(sim_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(sim_core PUBLIC cxx_std_17)
macro(add_gtest TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            sim_core
            GTest::gtest_main 
    )
    target_include_directories(${TARGET_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/include
    )
    include(GoogleTest)
    gtest_discover_tests(${TARGET_NAME})
endmacro()

macro(add_scenario TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            sim_core
            GTest::gtest
    )
    target_include_directories(${TARGET_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/include
    )
    include(GoogleTest)
    gtest_discover_tests(${TARGET_NAME}
        TEST_PREFIX "Scenario."
    )
endmacro()

add_scenario(scenario1_simple_transfer  scenarios/scenario1_simple_transfer.cpp)
add_scenario(scenario2_collision        scenarios/scenario2_collision.cpp)
add_scenario(scenario3_routing          scenarios/scenario3_routing.cpp)
add_scenario(scenario4_heavy_load       scenarios/scenario4_heavy_load.cpp)
add_scenario(scenario5_mixed_traffic    scenarios/scenario5_mixed_traffic.cpp)
add_scenario(scenario6_network_failure  scenarios/scenario6_network_failure.cpp)

add_gtest(unit_tests unit_tests.cpp)
