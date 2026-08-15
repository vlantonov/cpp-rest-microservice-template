#pragma once
#include "domain/ports/IReadinessChecker.hpp"

#include <drogon/HttpController.h>

#include <functional>

namespace infrastructure::http {

class HealthController
    : public drogon::HttpController<HealthController, false>
{
public:
    explicit HealthController(domain::ports::IReadinessChecker& checker);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::handleLive,  "/health/live",  drogon::Get);
    ADD_METHOD_TO(HealthController::handleReady, "/health/ready", drogon::Get);
    METHOD_LIST_END

    void handleLive (const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void handleReady(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    domain::ports::IReadinessChecker& checker_;
};

} // namespace infrastructure::http
