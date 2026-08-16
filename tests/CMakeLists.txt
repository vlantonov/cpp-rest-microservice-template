find_package(Catch2 3 REQUIRED)
find_package(fakeit  REQUIRED)
find_package(spdlog  REQUIRED)

add_executable(microservice_tests
    domain/GreetingUseCaseTest.cpp
    infrastructure/SpdlogLoggerTest.cpp
    # Compiled directly so the test binary does not inherit microservice_infra's
    # Drogon/Boost transitive link closure (libboost_test_exec_monitor conflicts
    # with Catch2's main).
    ${CMAKE_SOURCE_DIR}/src/infrastructure/logging/SpdlogLogger.cpp
)

target_include_directories(microservice_tests
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(microservice_tests
    PRIVATE microservice_domain
    PRIVATE spdlog::spdlog
    PRIVATE Catch2::Catch2WithMain
    PRIVATE fakeit::fakeit
)

target_compile_features(microservice_tests PRIVATE cxx_std_20)
target_compile_options(microservice_tests  PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_COVERAGE)
    target_compile_options(microservice_tests PRIVATE
        -g -O0 -fprofile-arcs -ftest-coverage
    )
    target_link_options(microservice_tests PRIVATE --coverage)
endif()

include(CTest)
include(Catch)
catch_discover_tests(microservice_tests)
