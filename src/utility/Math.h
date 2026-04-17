#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <algorithm>

/// Simple clamp function
/// It will clamp value between min and max, such that
/// a <= clamp(v, a, b) <= b
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(std::min(value, max), min);
}

#endif //MATH_H
