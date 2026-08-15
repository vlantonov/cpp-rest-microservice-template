#include <catch2/catch_test_macros.hpp>
#include <fakeit/catch/fakeit.hpp>

#include "domain/model/GreetingRequest.hpp"
#include "domain/ports/ILogger.hpp"
#include "domain/ports/IMetrics.hpp"
#include "domain/ports/ITracer.hpp"
#include "domain/use_cases/GreetingUseCase.hpp"

// ---------------------------------------------------------------------------
// Lightweight hand-written stubs for interfaces that return move-only types
// ---------------------------------------------------------------------------
namespace {

struct StubSpan final : public domain::ports::ISpan {
    void setAttribute(std::string_view, std::string_view) noexcept override {}
    void setStatus(bool, std::string_view) noexcept override {}
    std::string traceId() const noexcept override {
        return "00000000000000000000000000000000";
    }
    void end() noexcept override {}
};

struct StubTracer final : public domain::ports::ITracer {
    std::unique_ptr<domain::ports::ISpan> startSpan(
        std::string_view, std::string_view) noexcept override {
        return std::make_unique<StubSpan>();
    }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// GreetingRequest validation
// ---------------------------------------------------------------------------

TEST_CASE("GreetingRequest::create — validation", "[domain]") {
    SECTION("valid ASCII name succeeds") {
        auto req = domain::model::GreetingRequest::create("Alice");
        REQUIRE(req.has_value());
        REQUIRE(req->name() == "Alice");
    }

    SECTION("empty name returns nullopt") {
        REQUIRE_FALSE(domain::model::GreetingRequest::create("").has_value());
    }

    SECTION("name of exactly 64 chars succeeds") {
        REQUIRE(domain::model::GreetingRequest::create(std::string(64, 'A')).has_value());
    }

    SECTION("name longer than 64 chars returns nullopt") {
        REQUIRE_FALSE(domain::model::GreetingRequest::create(
            std::string(65, 'A')).has_value());
    }

    SECTION("non-printable ASCII character returns nullopt") {
        REQUIRE_FALSE(domain::model::GreetingRequest::create("Bad\x01Name").has_value());
    }
}

// ---------------------------------------------------------------------------
// GreetingUseCase — uses FakeIt mocks for ILogger and IMetrics
// ---------------------------------------------------------------------------

TEST_CASE("GreetingUseCase::execute — happy path", "[domain]") {
    using namespace fakeit;

    Mock<domain::ports::ILogger>  mock_logger;
    Mock<domain::ports::IMetrics> mock_metrics;
    StubTracer                    stub_tracer;

    When(Method(mock_logger,  log)).AlwaysReturn();
    When(Method(mock_metrics, recordRequest)).AlwaysReturn();
    When(Method(mock_metrics, recordLatency)).AlwaysReturn();

    domain::use_cases::GreetingUseCase use_case(
        mock_logger.get(), mock_metrics.get(), stub_tracer);

    auto req = domain::model::GreetingRequest::create("Alice");
    REQUIRE(req.has_value());

    const auto resp = use_case.execute(*req);
    REQUIRE(resp.message() == "Hello, Alice!");
}

TEST_CASE("GreetingUseCase::execute — logger called once", "[domain]") {
    using namespace fakeit;

    Mock<domain::ports::ILogger>  mock_logger;
    Mock<domain::ports::IMetrics> mock_metrics;
    StubTracer                    stub_tracer;

    When(Method(mock_logger,  log)).AlwaysReturn();
    When(Method(mock_metrics, recordRequest)).AlwaysReturn();
    When(Method(mock_metrics, recordLatency)).AlwaysReturn();

    domain::use_cases::GreetingUseCase use_case(
        mock_logger.get(), mock_metrics.get(), stub_tracer);

    auto req = domain::model::GreetingRequest::create("Bob");
    REQUIRE(req.has_value());
    use_case.execute(*req);

    Verify(Method(mock_logger, log)).Once();
}

TEST_CASE("GreetingUseCase::execute — metrics recorded", "[domain]") {
    using namespace fakeit;

    Mock<domain::ports::ILogger>  mock_logger;
    Mock<domain::ports::IMetrics> mock_metrics;
    StubTracer                    stub_tracer;

    When(Method(mock_logger,  log)).AlwaysReturn();
    When(Method(mock_metrics, recordRequest)).AlwaysReturn();
    When(Method(mock_metrics, recordLatency)).AlwaysReturn();

    domain::use_cases::GreetingUseCase use_case(
        mock_logger.get(), mock_metrics.get(), stub_tracer);

    auto req = domain::model::GreetingRequest::create("Carol");
    REQUIRE(req.has_value());
    use_case.execute(*req);

    Verify(Method(mock_metrics, recordRequest)).Once();
}
