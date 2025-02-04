#ifndef IW_MATH_HPP
#define IW_MATH_HPP 20250205L

#include <cstdint>

#include "macros.hpp"

#if IW_CPP_STD >= 202000L
#include <concepts>
#define IW_CONCEPT_INT std::integral
#else
#define IW_CONCEPT_INT class
#endif // IW_CPP_STD >= 202000L

namespace iw17 {

template<class T>
constexpr T clip(T val, T min, T max) noexcept {
    if (val <= min) {
        return min;
    }
    if (val >= max) {
        return max;
    }
    return val;
}

template<IW_CONCEPT_INT T>
constexpr T pydiv(T num, T den) noexcept {
    T quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quot - 1;
    }
    return quot;
}

template<IW_CONCEPT_INT T>
constexpr T pymod(T num, T den) noexcept {
    T rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return rem + den;
    }
    return rem;
}

template<IW_CONCEPT_INT T>
struct quotrem { // quot, rem
    T quot;
    T rem;
};

template<IW_CONCEPT_INT T>
constexpr quotrem<T> cdivmod(T num, T den) noexcept {
    T quot = num / den, rem = num % den;
    return quotrem<T>{quot, rem};
}

template<IW_CONCEPT_INT T>
constexpr quotrem<T> pydivmod(T num, T den) noexcept {
    T quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quotrem<T>{quot - 1, rem + den};
    }
    return quotrem<T>{quot, rem};
}

constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;
constexpr double HALF_PI = PI / 2.0;

constexpr double cos_in45d(double x) noexcept {
    double x2 = x * x;
    double cosx = 1.0;
    if (x > 0.4069) {
        cosx = 1.0 - cosx * x2 / (16.0 * 15.0);
        cosx = 1.0 - cosx * x2 / (14.0 * 13.0);
    }
    if (x > 0.1036) {
        cosx = 1.0 - cosx * x2 / (12.0 * 11.0);
        cosx = 1.0 - cosx * x2 / (10.0 * 9.0);
    }
    if (x > 1.7e-4) {
        cosx = 1.0 - cosx * x2 / (8.0 * 7.0);
        cosx = 1.0 - cosx * x2 / (6.0 * 5.0);
        cosx = 1.0 - cosx * x2 / (4.0 * 3.0);
    }
    cosx = 1.0 - cosx * x2 / 2.0;
    return cosx;
}

constexpr double sin_in45d(double x) noexcept {
    double x2 = x * x;
    double sinc = 1.0;
    if (x > 0.5024) {
        sinc = 1.0 - sinc * x2 / (17.0 * 16.0);
        sinc = 1.0 - sinc * x2 / (15.0 * 14.0);
    }
    if (x > 0.1348) {
        sinc = 1.0 - sinc * x2 / (13.0 * 12.0);
        sinc = 1.0 - sinc * x2 / (11.0 * 10.0);
    }
    if (x > 2.7e-4) {
        sinc = 1.0 - sinc * x2 / (9.0 * 8.0);
        sinc = 1.0 - sinc * x2 / (7.0 * 6.0);
        sinc = 1.0 - sinc * x2 / (5.0 * 4.0);
    }
    sinc = 1.0 - sinc * x2 / (3.0 * 2.0);
    return x * sinc;
}

constexpr double tan_in45d(double x) noexcept {
    return sin_in45d(x) / cos_in45d(x);
}

constexpr double cot_in45d(double x) noexcept {
    return cos_in45d(x) / sin_in45d(x);
}

constexpr double cos(double x) noexcept {
    int8_t sign = 1;
    if (x < 0.0) {
        x = -x;
    }
    if (x >= TWO_PI) {
        x -= TWO_PI * int64_t(x / TWO_PI);
    }
    if (x > PI) {
        x = TWO_PI - x;
    }
    if (x > HALF_PI) {
        x = PI - x;
        sign = -sign;
    }
    if (x > PI / 4.0) {
        return sign * sin_in45d(HALF_PI - x);
    }
    return sign * cos_in45d(x);
}

constexpr double sin(double x) noexcept {
    int8_t sign = 1;
    if (x < 0.0) {
        x = -x;
        sign = -sign;
    }
    if (x >= TWO_PI) {
        x -= TWO_PI * int64_t(x / TWO_PI);
    }
    if (x > PI) {
        x -= PI;
        sign = -sign;
    }
    if (x > HALF_PI) {
        x = PI - x;
    }
    if (x > PI / 4.0) {
        return sign * cos_in45d(HALF_PI - x);
    }
    return sign * sin_in45d(x);
}

constexpr double tan(double x) noexcept {
    int8_t sign = 1;
    if (x < 0.0) {
        x = -x;
        sign = -sign;
    }
    if (x >= PI) {
        x -= PI * int64_t(x / PI);
    }
    if (x > HALF_PI) {
        x = PI - x;
        sign = -sign;
    }
    if (x > PI / 4.0) {
        return sign * cot_in45d(HALF_PI - x);
    }
    return sign * tan_in45d(x);
}

constexpr double cot(double x) noexcept {
    int8_t sign = 1;
    if (x < 0.0) {
        x = -x;
        sign = -sign;
    }
    if (x >= PI) {
        x -= PI * int64_t(x / PI);
    }
    if (x > HALF_PI) {
        x = PI - x;
        sign = -sign;
    }
    if (x > PI / 4.0) {
        return sign * tan_in45d(HALF_PI - x);
    }
    return sign * cot_in45d(x);
}

} // namespace iw17

#endif // IW_MATH_HPP