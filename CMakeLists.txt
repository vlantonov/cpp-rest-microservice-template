cmake_minimum_required(VERSION 3.21)
project(cpp-rest-microservice-template
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

option(ENABLE_COVERAGE "Enable coverage instrumentation" OFF)

add_subdirectory(src/domain)
add_subdirectory(src/infrastructure)
add_subdirectory(src/application)
add_subdirectory(tests)

# ---------------------------------------------------------------------------
# Coverage targets (pattern from vladiant/test_cpp_ci)
# ---------------------------------------------------------------------------
find_program(GCOV_PATH gcov)
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(LCOV_PATH AND GCOV_PATH)
    set(covname cov)

    add_custom_target(cov_data
        ${LCOV_PATH} --directory . --zerocounters
        COMMAND microservice_tests
        COMMAND ${LCOV_PATH} --directory . --capture --output-file ${covname}.info
        COMMAND ${LCOV_PATH} --remove ${covname}.info
            '${CMAKE_SOURCE_DIR}/tests/*'
            '${CMAKE_SOURCE_DIR}/src/infrastructure/*'
            '*/.conan2/*'
            '/usr/*'
            --ignore-errors unused
            --output-file ${covname}.info.cleaned
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage data"
    )

    if(GENHTML_PATH)
        add_custom_target(cov
            ${LCOV_PATH} --directory . --zerocounters
            COMMAND microservice_tests
            COMMAND ${LCOV_PATH} --directory . --capture --output-file ${covname}.info
            COMMAND ${LCOV_PATH} --remove ${covname}.info
                '${CMAKE_SOURCE_DIR}/tests/*'
                '${CMAKE_SOURCE_DIR}/src/infrastructure/*'
                '*/.conan2/*'
                '/usr/*'
                --ignore-errors unused
                --output-file ${covname}.info.cleaned
            COMMAND ${GENHTML_PATH} -o ${covname} ${covname}.info.cleaned
            COMMAND ${CMAKE_COMMAND} -E remove ${covname}.info ${covname}.info.cleaned
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report"
        )
    endif()
endif()
