add_library(microservice_domain STATIC
    use_cases/GreetingUseCase.cpp
)

target_include_directories(microservice_domain
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(microservice_domain PUBLIC cxx_std_20)
target_compile_options(microservice_domain PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_COVERAGE)
    target_compile_options(microservice_domain PRIVATE
        -g -O0 -fprofile-arcs -ftest-coverage
    )
    target_link_options(microservice_domain PRIVATE --coverage)
endif()
