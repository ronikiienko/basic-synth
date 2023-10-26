#pragma once

#include <cmath>
#include <iostream>
#include "Angle.h"

template <typename T>
class Vector2 {
private:
    constexpr explicit Vector2(T x = 0, T y = 0) : x(x), y(y) {}
public:
    T x, y;

    constexpr static Vector2 fromCartesian(T x = 0, T y = 0) {
        return Vector2{x, y};
    }

    constexpr static Vector2 fromPolar(T radius, const Angle<T> &angle) {
        T angleRadians = angle.getRadians();

        T x = radius * std::cos(angleRadians);
        T y = radius * std::sin(angleRadians);

        return Vector2{x, y};
    }

    constexpr Vector2 operator-(const Vector2 &other) const {
        return Vector2{x - other.x, y - other.y};
    }

    constexpr Vector2 operator+(const Vector2 &other) const {
        return Vector2{x + other.x, y + other.y};
    }

    constexpr Vector2 operator*(T scalar) const {
        return Vector2{x * scalar, y * scalar};
    }

    constexpr Vector2 operator/(T scalar) const {
        // TODO remove safety checks
        if (scalar == 0) {
            throw std::runtime_error("Trying to divide by 0 (/)");
        }
        return Vector2{x / scalar, y / scalar};
    }

    [[nodiscard]] constexpr T dot(const Vector2 &other) const {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr T magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    [[nodiscard]] constexpr T magnitude2() const {
        return x * x + y * y;
    }

    [[nodiscard]] constexpr Vector2 normalize() const {
        T mag = magnitude();
        return Vector2{x / mag, y / mag};
    }

    void log() const {
        std::cout << '\n' << "X: " << x << " " << "Y: " << y << '\n';
    }

    // Add the += operator
    constexpr Vector2 &operator+=(const Vector2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Add the -= operator
    constexpr Vector2 &operator-=(const Vector2 &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Add the -= operator
    constexpr Vector2 &operator/=(const T scalar) {
        // TODO remove safety checks
        if (scalar == 0) {
            throw std::runtime_error("Trying to divide by 0 (/=)");
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Add the -= operator
    constexpr Vector2 &operator*=(const T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2d = Vector2<double>;