#pragma once
#include <optional>
#include <string>

namespace domain::model {

/// Validated inbound name for a greeting request.
/// Use the factory; the constructor is private.
class GreetingRequest {
public:
    /// Returns nullopt if name is empty, longer than 64 chars, or contains
    /// non-printable ASCII (outside 0x20–0x7E).
    [[nodiscard]] static std::optional<GreetingRequest> create(std::string name) {
        if (name.empty() || name.size() > 64) {
            return std::nullopt;
        }
        for (unsigned char c : name) {
            if (c < 0x20 || c > 0x7E) {
                return std::nullopt;
            }
        }
        return GreetingRequest{std::move(name)};
    }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }

private:
    explicit GreetingRequest(std::string name) : name_(std::move(name)) {}

    std::string name_;
};

} // namespace domain::model
