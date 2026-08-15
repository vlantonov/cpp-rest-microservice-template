#pragma once
#include <string>

namespace domain::model {

class GreetingResponse {
public:
    explicit GreetingResponse(std::string message) : message_(std::move(message)) {}

    [[nodiscard]] const std::string& message() const noexcept { return message_; }

private:
    std::string message_;
};

} // namespace domain::model
