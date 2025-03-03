#ifndef IW_MATH_HPP
#define IW_MATH_HPP 20250205L

#include <cstdint>
#include <type_traits>

namespace iw17::math {

template<class T>
constexpr T round(double val) noexcept {
    static_assert(std::is_integral_v<T>);
    return val + (val < 0.0 ? -0.5 : 0.5);
}

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

template<class T>
constexpr T pydiv(T num, T den) noexcept {
    static_assert(std::is_integral_v<T>);
    T quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quot - 1;
    }
    return quot;
}

template<class T>
constexpr T pymod(T num, T den) noexcept {
    static_assert(std::is_integral_v<T>);
    T rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return rem + den;
    }
    return rem;
}

template<class T>
struct quotrem { // quot, rem
    static_assert(std::is_integral_v<T>);
    T quot, rem;
};

template<class T>
constexpr quotrem<T> cdivmod(T num, T den) noexcept {
    T quot = num / den, rem = num % den;
    return quotrem<T>{quot, rem};
}

template<class T>
constexpr quotrem<T> pydivmod(T num, T den) noexcept {
    T quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quotrem<T>{quot - 1, rem + den};
    }
    return quotrem<T>{quot, rem};
}

// ccos(x) = cos(2 PI x)
constexpr double ccos_in45d(double x) noexcept {
    constexpr double TAU_SQ = 39.47841760435743447534;
    double x2 = x * x;
    double cosx = 1.0;
    if (x > 0.042641584944336657) {
        cosx = 1.0 - cosx * x2 * (TAU_SQ / (10.0 * 9.0));
        cosx = 1.0 - cosx * x2 * (TAU_SQ / (8.0 * 7.0));
    }
    if (x > 0.001784510982635752) {
        cosx = 1.0 - cosx * x2 * (TAU_SQ / (6.0 * 5.0));
        cosx = 1.0 - cosx * x2 * (TAU_SQ / (4.0 * 3.0));
    }
    cosx = 1.0 - cosx * x2 * (TAU_SQ / 2.0);
    return cosx;
}

// csin(x) = sin(2 PI x)
constexpr double csin_in45d(double x) noexcept {
    constexpr double TAU = 6.283185307179586476925;
    constexpr double TAU_SQ = 39.47841760435743447534;
    double x2 = x * x;
    double sinc = TAU;
    if (x > 0.04596792854596641) {
        sinc = TAU - sinc * x2 * (TAU_SQ / (11.0 * 10.0));
        sinc = TAU - sinc * x2 * (TAU_SQ / (9.0 * 8.0));
    }
    if (x > 0.00179037617018252) {
        sinc = TAU - sinc * x2 * (TAU_SQ / (7.0 * 6.0));
        sinc = TAU - sinc * x2 * (TAU_SQ / (5.0 * 4.0));
    }
    sinc = TAU - sinc * x2 * (TAU_SQ / (3.0 * 2.0));
    return x * sinc;
}

// ccos(x) = cos(2 PI x)
constexpr double ccos(double x) noexcept {
    int8_t sign = 1;
    x -= int64_t(x) - (x < 0.0);
    if (x > 0.5) {
        x = 1.0 - x;
    }
    if (x > 0.25) {
        x = 0.5 - x;
        sign = -sign;
    }
    if (x <= 0.125) {
        return sign * ccos_in45d(x);
    }
    return sign * csin_in45d(0.25 - x);
}

// csin(x) = sin(2 PI x)
constexpr double csin(double x) noexcept {
    int8_t sign = 1;
    x -= int64_t(x) - (x < 0.0);
    if (x > 0.5) {
        x = 1.0 - x;
        sign = -sign;
    }
    if (x > 0.25) {
        x = 0.5 - x;
    }
    if (x <= 0.125) {
        return sign * csin_in45d(x);
    }
    return sign * ccos_in45d(0.25 - x);
}

} // namespace iw17

#endif // IW_MATH_HPP
