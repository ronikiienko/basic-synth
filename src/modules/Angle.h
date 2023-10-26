#pragma once

#include <cmath>

template <typename T>
class Angle {
private:
    T degrees;

    constexpr explicit Angle(T degrees) : degrees(degrees) {}

public:
    static Angle fromDegrees(T degrees) {
        return Angle{degrees};
    }

    static Angle fromRadians(T radians) {
        return Angle{radians * 180.0f / static_cast<T>(M_PI)};
    }

    [[nodiscard]] T getDegrees() const {
        return degrees;
    }

    [[nodiscard]] T getRadians() const {
        return degrees * static_cast<T>(M_PI) / 180.0f;
    }
};
