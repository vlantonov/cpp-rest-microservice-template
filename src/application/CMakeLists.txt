add_executable(microservice_app
    main.cpp
)

target_include_directories(microservice_app
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(microservice_app
    PRIVATE microservice_domain
    PRIVATE microservice_infra
)

target_compile_features(microservice_app PRIVATE cxx_std_20)
target_compile_options(microservice_app PRIVATE -Wall -Wextra -Wpedantic)
