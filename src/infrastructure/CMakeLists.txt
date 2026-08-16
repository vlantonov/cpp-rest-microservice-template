find_package(Drogon          REQUIRED)
find_package(spdlog          REQUIRED)
find_package(opentelemetry-cpp REQUIRED)
find_package(prometheus-cpp  REQUIRED)

add_library(microservice_infra STATIC
    http/GreetingController.cpp
    http/HealthController.cpp
    http/MetricsController.cpp
    logging/SpdlogLogger.cpp
    metrics/PrometheusMetrics.cpp
    tracing/OtelTracer.cpp
)

target_link_libraries(microservice_infra
    PUBLIC  microservice_domain
    PRIVATE Drogon::Drogon
    PRIVATE spdlog::spdlog
    PRIVATE opentelemetry-cpp::opentelemetry-cpp
    PUBLIC  prometheus-cpp::core
)

target_compile_features(microservice_infra PUBLIC cxx_std_20)
target_compile_options(microservice_infra PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_COVERAGE)
    target_compile_options(microservice_infra PRIVATE
        -g -O0 -fprofile-arcs -ftest-coverage
    )
    target_link_options(microservice_infra PRIVATE --coverage)
endif()
