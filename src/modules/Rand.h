#pragma once

#include <random>

class NumberGenerator {
protected:
    std::random_device rd;
    std::mt19937 gen;

    NumberGenerator() : gen(rd()) {};

    explicit NumberGenerator(int seed) : gen(seed) {};
};

template<typename T>
class RealNumberGenerator : NumberGenerator {
private:
    std::uniform_real_distribution<T> distribution{0, 1};
public:
    explicit RealNumberGenerator(int seed) : NumberGenerator(seed) {}

    T get() {
        return distribution(gen);
    }

    T getInRange(T min, T max) {
        return min + get() * (max - min);
    }
};

template<typename T>
class IntNumberGenerator : NumberGenerator {
public:
    explicit IntNumberGenerator(int seed) : NumberGenerator(seed) {}
    explicit IntNumberGenerator() : NumberGenerator() {}

    T getInRange(T min, T max) {
        std::uniform_int_distribution<T> distribution(min, max);
        return distribution(gen);
    }
};

using RNGf = RealNumberGenerator<float>;
using RNGd = RealNumberGenerator<double>;
using RNGi = IntNumberGenerator<int>;
using RNGs = IntNumberGenerator<short>;
