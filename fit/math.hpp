#ifndef IW_MATH_HPP
#define IW_MATH_HPP 20250205L

#include <cstdint>

#include "prestd.hpp"

namespace iw17::math {

inline namespace literal {

using _uintmax = unsigned long long;

constexpr int16_t operator""_i16(_uintmax n) noexcept {
    return static_cast<int16_t>(n);
}

constexpr uint16_t operator""_u16(_uintmax n) noexcept {
    return static_cast<uint16_t>(n);
}

constexpr int32_t operator""_i32(_uintmax n) noexcept {
    return static_cast<int32_t>(n);
}

constexpr uint32_t operator""_u32(_uintmax n) noexcept {
    return static_cast<uint32_t>(n);
}

constexpr int64_t operator""_i64(_uintmax n) noexcept {
    return static_cast<int64_t>(n);
}

constexpr uint64_t operator""_u64(_uintmax n) noexcept {
    return static_cast<uint64_t>(n);
}

} // namespace literal

template <class T>
constexpr T clip(T val, T min, T max) noexcept {
    if (val <= min) {
        return min;
    }
    if (val >= max) {
        return max;
    }
    return val;
}

template <class Int>
constexpr Int pydiv(Int num, Int den) noexcept {
    Int quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quot - 1;
    }
    return quot;
}

template <class Int>
constexpr Int pymod(Int num, Int den) noexcept {
    Int rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return rem + den;
    }
    return rem;
}

template <class Int>
struct quotrem { // quot, rem
    Int quot, rem;
};

template <class Int>
constexpr quotrem<Int> cdivmod(Int num, Int den) noexcept {
    Int quot = num / den, rem = num % den;
    return quotrem<Int>{quot, rem};
}

template <class Int>
constexpr quotrem<Int> pydivmod(Int num, Int den) noexcept {
    Int quot = num / den, rem = num % den;
    if (rem != 0 && (den < 0) != (rem < 0)) {
        return quotrem<Int>{quot - 1, rem + den};
    }
    return quotrem<Int>{quot, rem};
}

enum class fix64: int64_t {};

namespace _fix {

constexpr int64_t FBITS = 32;
constexpr int64_t SCALE = 1_i64 << FBITS;
constexpr int64_t FPART = SCALE - 1;

} // namespace _data

constexpr fix64 fill_fix64(int64_t v) noexcept {
    return static_cast<fix64>(v);
}

constexpr int64_t pour_int64(fix64 a) noexcept {
    return static_cast<int64_t>(a);
}

constexpr fix64 make_fix64(int64_t n) noexcept {
    return fill_fix64(n << _fix::FBITS);
}

constexpr fix64 make_fix64(double d) noexcept {
    double dval = _fix::SCALE * d;
    int64_t ival = dval + (d < 0.0 ? -0.5 : 0.5);
    return fill_fix64(ival);
}

constexpr int64_t fast_int(fix64 a) noexcept {
    int64_t av = pour_int64(a);
    return av >> _fix::FBITS;
}

constexpr int64_t safe_int(fix64 a) noexcept {
    constexpr int64_t HALF = _fix::SCALE / 2;
    int64_t av = pour_int64(a);
    return (av >> _fix::FBITS) + bool(av & HALF);
}

constexpr double show_double(fix64 a) noexcept {
    int64_t av = pour_int64(a);
    return av / double(_fix::SCALE);
}

inline namespace literal {

using _floatmax = long double;

constexpr fix64 operator""_fix(_uintmax n) noexcept {
    return make_fix64(int64_t(n));
}

constexpr fix64 operator""_fix(_floatmax d) noexcept {
    _floatmax dval = _fix::SCALE * d;
    int64_t ival = dval + (d < 0.0L ? -0.5L : 0.5L);
    return fill_fix64(ival);
}

} // namespace literal

constexpr fix64 operator+(fix64 a) noexcept {
    return a;
}

constexpr fix64 operator-(fix64 a) noexcept {
    int64_t av = pour_int64(a);
    return fill_fix64(-av);
}

constexpr fix64 operator+(fix64 a, fix64 b) noexcept {
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    return fill_fix64(av + bv);
}

constexpr fix64 operator-(fix64 a, fix64 b) noexcept {
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    return fill_fix64(av - bv);
}

constexpr fix64 operator>>(fix64 a, int64_t n) noexcept {
    int64_t av = pour_int64(a);
    return fill_fix64(av >> n);
}

constexpr fix64 operator<<(fix64 a, int64_t n) noexcept {
    int64_t av = pour_int64(a);
    return fill_fix64(av << n);
}

constexpr fix64 operator*(fix64 a, int64_t n) noexcept {
    uint64_t av = pour_int64(a);
    return fill_fix64(av * n);
}

constexpr fix64 operator*(int64_t n, fix64 b) noexcept {
    uint64_t bv = pour_int64(b);
    return fill_fix64(n * bv);
}

// neglects overflow and truncates fractional part
constexpr fix64 fast_mul(fix64 a, fix64 b) noexcept {
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    int64_t pv = av * bv; // overflowed bits discarded
    return fill_fix64(pv >> _fix::FBITS);
}

// neglects overflows and rounds off fractional part
constexpr fix64 tiny_mul(fix64 a, fix64 b) noexcept {
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    int64_t pv = av * bv + _fix::SCALE / 2;
    return fill_fix64(pv >> _fix::FBITS);
}

// considers overflows for values possibly reaching 1/2
constexpr fix64 fair_mul(fix64 a, fix64 b) noexcept {
    int64_t av = pour_int64(a), bv = pour_int64(b);
    uint64_t ah = av >> _fix::FBITS, al = av & _fix::FPART;
    uint64_t bh = bv >> _fix::FBITS, bl = bv & _fix::FPART;
    uint64_t hi = av * bh + ah * bl;
    uint64_t lo = (al * bl) >> _fix::FBITS;
    return fill_fix64(hi + lo);
}

// considers overflows and rounds off fractional part
constexpr fix64 safe_mul(fix64 a, fix64 b) noexcept {
    constexpr uint64_t HALF = _fix::SCALE / 2;
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    uint64_t ah = av >> _fix::FBITS, al = av & _fix::FPART;
    uint64_t bh = bv >> _fix::FBITS, bl = bv & _fix::FPART;
    uint64_t hi = av * bh + ah * bl, lv = al * bl;
    uint64_t lo = (lv >> _fix::FBITS) + bool(lv & HALF);
    return fill_fix64(hi + lo);
}

constexpr fix64 operator/(fix64 a, int64_t n) noexcept {
    int64_t av = pour_int64(a);
    return fill_fix64(av / n);
}

// small numerator and small denominator
constexpr fix64 fast_div(int64_t a, int64_t b) noexcept {
    int64_t av = uint64_t(a) << _fix::FBITS;
    return fill_fix64(av / b);
}

// large numerator and small denominator
constexpr fix64 fair_div(int64_t a, int64_t b) noexcept {
    auto [quot, rem] = cdivmod<int64_t>(a, b);
    return make_fix64(quot) + fast_div(rem, b);
}

namespace _tiny {

// cos(PI/2 * x) for 0 <= x <= 1/8
constexpr fix64 cosq(fix64 x) noexcept {
    // coefficients of Taylor series
    constexpr fix64 COEFS[] = {
        make_fix64(+1.000000000000000000000e-0),
        make_fix64(-1.233700550136169827354e-0),
        make_fix64(+2.536695079010480136366e-1),
        make_fix64(-2.086348076335296087305e-2),
        make_fix64(+9.192602748394265802417e-4),
        make_fix64(-2.520204237306060548105e-5),
        make_fix64(+4.710874778818171503670e-7),
    };
    // using Qin J.S.'s method
    fix64 x2 = fast_mul(x, x), cosx = COEFS[6];
    cosx = fast_mul(x2, cosx) + COEFS[5];
    cosx = fast_mul(x2, cosx) + COEFS[4];
    cosx = fast_mul(x2, cosx) + COEFS[3];
    cosx = fast_mul(x2, cosx) + COEFS[2];
    cosx = fast_mul(x2, cosx) + COEFS[1];
    cosx = fast_mul(x2, cosx) + COEFS[0];
    return cosx;
}

// sin(PI/2 * x) for 0 <= x <= 1/8
constexpr fix64 sinq(fix64 x) noexcept {
    // coefficients of Taylor series
    constexpr fix64 COEFS[] = {
        make_fix64(+1.570796326794896619231e-0),
        make_fix64(-6.459640975062462536558e-1),
        make_fix64(+7.969262624616704512051e-2),
        make_fix64(-4.681754135318688100685e-3),
        make_fix64(+1.604411847873598218727e-4),
        make_fix64(-3.598843235212085340459e-6),
        make_fix64(+5.692172921967926811775e-8),
    };
    // using Qin J.S.'s method
    fix64 x2 = fast_mul(x, x), sinc = COEFS[6];
    sinc = fast_mul(x2, sinc) + COEFS[5];
    sinc = fast_mul(x2, sinc) + COEFS[4];
    sinc = fast_mul(x2, sinc) + COEFS[3];
    sinc = fast_mul(x2, sinc) + COEFS[2];
    sinc = fast_mul(x2, sinc) + COEFS[1];
    sinc = fast_mul(x2, sinc) + COEFS[0];
    uint64_t xv = pour_int64(x), cv = pour_int64(sinc);
    uint64_t sv = (xv * cv) >> _fix::FBITS;
    return fill_fix64(sv);
}

} // namespace _tiny

// cosq(x) = cos(PI/2 * x)
constexpr fix64 cosq(fix64 x) noexcept {
    constexpr int64_t SBITS = _fix::FBITS - 1;
    constexpr int64_t SPART = (1_i64 << SBITS) - 1;
    // PI/4 rad, like a slice of pizza
    constexpr fix64 SLICE = fill_fix64(SPART + 1);
    uint64_t xv = pour_int64(x);
    x = fill_fix64(xv & SPART);
    switch ((xv >> SBITS) & 07) {
        case 00: return +_tiny::cosq(x);
        case 01: return +_tiny::sinq(SLICE - x);
        case 02: return -_tiny::sinq(x);
        case 03: return -_tiny::cosq(SLICE - x);
        case 04: return -_tiny::cosq(x);
        case 05: return -_tiny::sinq(SLICE - x);
        case 06: return +_tiny::sinq(x);
        case 07: return +_tiny::cosq(SLICE - x);
        default: prestd::unreachable();
    }
}

// sinq(x) = sin(PI/2 * x)
constexpr fix64 sinq(fix64 x) noexcept {
    constexpr int64_t SBITS = _fix::FBITS - 1;
    constexpr int64_t SPART = (1_i64 << SBITS) - 1;
    // 1/8 of a whole round, like a slice of pizza
    constexpr fix64 SLICE = fill_fix64(SPART + 1);
    uint64_t xv = pour_int64(x);
    x = fill_fix64(xv & SPART);
    switch ((xv >> SBITS) & 07) {
        case 00: return +_tiny::sinq(x);
        case 01: return +_tiny::cosq(SLICE - x);
        case 02: return +_tiny::cosq(x);
        case 03: return +_tiny::sinq(SLICE - x);
        case 04: return -_tiny::sinq(x);
        case 05: return -_tiny::cosq(SLICE - x);
        case 06: return -_tiny::cosq(x);
        case 07: return -_tiny::sinq(SLICE - x);
        default: prestd::unreachable();
    }
}

} // namespace iw17::math

#endif // IW_MATH_HPP
