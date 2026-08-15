#pragma once

namespace domain::ports {

class IReadinessChecker {
public:
    virtual ~IReadinessChecker() = default;
    [[nodiscard]] virtual bool isReady() const noexcept = 0;
};

} // namespace domain::ports
