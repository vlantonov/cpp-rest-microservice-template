#include "application/Config.hpp"
#include "application/DependencyContainer.hpp"
#include "infrastructure/http/GreetingController.hpp"
#include "infrastructure/http/HealthController.hpp"
#include "infrastructure/http/MetricsController.hpp"

#include <drogon/drogon.h>

int main() {
    application::Config             config;
    application::DependencyContainer container(config);

    drogon::app()
        .registerController(
            std::make_shared<infrastructure::http::GreetingController>(
                container.greetingUseCase(),
                container.tracer(),
                container.logger(),
                container.metrics()))
        .registerController(
            std::make_shared<infrastructure::http::HealthController>(
                container.readinessChecker()))
        .registerController(
            std::make_shared<infrastructure::http::MetricsController>(
                container.metrics()))
        .addListener("0.0.0.0", config.port)
        .run();

    return 0;
}
