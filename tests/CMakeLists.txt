include(FetchContent)

find_package(GTest REQUIRED)

add_executable(CircuitECS_test
    ecs/CircuitECS_test.cpp
)

add_executable(Snapshot_test
    snapshot/Snapshot_test.cpp
)

target_link_libraries(CircuitECS_test
    PRIVATE
        circuit
        GTest::gtest
        GTest::gtest_main
)

target_link_libraries(Snapshot_test
    PRIVATE
        circuit
        GTest::gtest
        GTest::gtest_main
)


include(GoogleTest)
gtest_discover_tests(CircuitECS_test)
gtest_discover_tests(Snapshot_test)