cmake_minimum_required(VERSION 3.13)
project(net_simulation)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Werror -Wpedantic -pedantic-errors -Werror=switch)

include_directories(include)

set(SOURCE_FILES
        src/package.cpp
        src/storage_types.cpp
        src/factory.cpp
        src/nodes.cpp
        src/helpers.cpp
        src/reports.cpp
        src/simulation.cpp
        )


add_executable(netsim ${SOURCE_FILES})


#Google TEST INITIALIZATION
add_subdirectory(googletest-master)

set(SOURCES_FILES_TESTS
        netsim_tests/test/test_package.cpp
        netsim_tests/test/test_storage_types.cpp
        netsim_tests/test/test_nodes.cpp
        netsim_tests/test/test_Factory.cpp
        netsim_tests/test/test_factory_io.cpp
        netsim_tests/test/test_reports.cpp
        netsim_tests/test/test_simulate.cpp
        netsim_tests/test/main_gtest.cpp
        )

add_executable(netsim__test ${SOURCE_FILES} ${SOURCES_FILES_TESTS} netsim_tests/test/main_gtest.cpp)

target_compile_definitions(netsim__test PUBLIC EXERCISE_ID=REPORTING)


target_include_directories(netsim__test PUBLIC
        googletest-master/googlemock/include
        googletest-master/googletest/include
        )

target_link_libraries(netsim__test gmock)