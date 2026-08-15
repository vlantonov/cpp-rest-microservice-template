#pragma once
#include "domain/ports/IMetrics.hpp"

#include <drogon/HttpController.h>

#include <functional>

namespace infrastructure::http {

class MetricsController
    : public drogon::HttpController<MetricsController, false>
{
public:
    explicit MetricsController(domain::ports::IMetrics& metrics);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MetricsController::handleMetrics, "/metrics", drogon::Get);
    METHOD_LIST_END

    void handleMetrics(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    domain::ports::IMetrics& metrics_;
};

} // namespace infrastructure::http
