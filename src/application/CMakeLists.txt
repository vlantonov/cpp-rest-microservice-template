find_package(Drogon REQUIRED)

add_executable(microservice_app
    main.cpp
)

target_include_directories(microservice_app
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(microservice_app
    PRIVATE microservice_domain
    PRIVATE microservice_infra
    PRIVATE Drogon::Drogon
)

target_compile_features(microservice_app PRIVATE cxx_std_20)
target_compile_options(microservice_app PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_COVERAGE)
    target_compile_options(microservice_app PRIVATE
        -g -O0 -fprofile-arcs -ftest-coverage
    )
    target_link_options(microservice_app PRIVATE --coverage)
endif()
