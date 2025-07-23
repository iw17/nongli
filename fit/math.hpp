#ifndef IW_MATH_HPP
#define IW_MATH_HPP 20250205L

#include <cstdint>
#include <type_traits>

#include "prestd.hpp"

namespace iw17::math {

inline namespace literal {

using namespace prestd::literal;

} // namespace literal

template <class Out, class In>
constexpr std::enable_if_t<
    std::is_integral_v<Out> &&
    std::is_arithmetic_v<In>,
Out> half_up(In fval) noexcept {
    if constexpr (std::is_integral_v<In>) {
        return Out(fval);
    }
    constexpr In HALF = In(0.5);
    intmax_t ival = intmax_t(fval + HALF);
    uintmax_t over = ival - fval > HALF;
    return static_cast<Out>(ival - over);
}

template <class Int>
constexpr std::enable_if_t<
    std::is_integral_v<Int>,
Int> pydiv(Int num, Int den) noexcept {
    if constexpr (std::is_unsigned_v<Int>) {
        return num / den;
    }
    Int quot = num / den, rem = num % den;
    bool cyc = rem != 0 && (den < 0) != (rem < 0);
    return quot - cyc;
}

template <class Int>
constexpr std::enable_if_t<
    std::is_integral_v<Int>,
Int> pymod(Int num, Int den) noexcept {
    if constexpr (std::is_unsigned_v<Int>) {
        return num % den;
    }
    Int rem = num % den;
    bool cyc = rem != 0 && (den < 0) != (rem < 0);
    return rem + den * cyc;
}

template <class Int>
struct quotrem { // quot, rem
    Int quot, rem;
};

template <class Int>
constexpr std::enable_if_t<
    std::is_integral_v<Int>,
quotrem<Int>> cdivmod(Int num, Int den) noexcept {
    Int quot = num / den, rem = num % den;
    return quotrem<Int>{quot, rem};
}

template <class Int>
constexpr std::enable_if_t<
    std::is_integral_v<Int>,
quotrem<Int>> pydivmod(Int num, Int den) noexcept {
    Int quot = num / den, rem = num % den;
    if constexpr (std::is_unsigned_v<Int>) {
        return quotrem<Int>{quot, rem};
    }
    bool cyc = rem != 0 && (den < 0) != (rem < 0);
    return quotrem<Int>{quot - cyc, rem + den * cyc};
}

enum class fix64: int64_t {};

namespace fix {

constexpr int64_t FBITS = 32;
constexpr int64_t SCALE = 1_u64 << FBITS;
constexpr int64_t FPART = SCALE - 1;

} // namespace fix

constexpr fix64 fill_fix64(int64_t v) noexcept {
    return static_cast<fix64>(v);
}

constexpr int64_t pour_int64(fix64 a) noexcept {
    return static_cast<int64_t>(a);
}

template <class Int>
constexpr std::enable_if_t<
    std::is_integral_v<Int>,
fix64> make_fix64(Int n) noexcept {
    int64_t nv = uint64_t(n) << fix::FBITS;
    return fill_fix64(nv);
}

template <class Float>
constexpr std::enable_if_t<
    std::is_floating_point_v<Float>,
fix64> make_fix64(Float d) noexcept {
    Float dval = fix::SCALE * d;
    int64_t ival = half_up<int64_t>(dval);
    return fill_fix64(ival);
}

constexpr int64_t fast_int(fix64 a) noexcept {
    int64_t av = pour_int64(a);
    return av >> fix::FBITS;
}

constexpr int64_t safe_int(fix64 a) noexcept {
    constexpr int64_t HALF = fix::SCALE / 2;
    int64_t av = pour_int64(a);
    return (av >> fix::FBITS) + bool(av & HALF);
}

constexpr double show_double(fix64 a) noexcept {
    int64_t av = pour_int64(a);
    return av / double(fix::SCALE);
}

inline namespace literal {

using prestd::literal::uintmax;

using floatmax = long double;

constexpr fix64 operator""_fix(uintmax n) noexcept {
    return make_fix64(n);
}

constexpr fix64 operator""_fix(floatmax d) noexcept {
    return make_fix64(d);
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
    uint64_t av = pour_int64(a);
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
    return fill_fix64(pv >> fix::FBITS);
}

// neglects overflows and rounds off fractional part
constexpr fix64 tiny_mul(fix64 a, fix64 b) noexcept {
    uint64_t av = pour_int64(a), bv = pour_int64(b);
    int64_t pv = av * bv + fix::SCALE / 2;
    return fill_fix64(pv >> fix::FBITS);
}

// considers overflows for values possibly reaching 1/2
constexpr fix64 fair_mul(fix64 a, fix64 b) noexcept {
    int64_t av = pour_int64(a), bv = pour_int64(b);
    uint64_t ah = av >> fix::FBITS, al = av & fix::FPART;
    uint64_t bh = bv >> fix::FBITS, bl = bv & fix::FPART;
    uint64_t hi = av * bh + ah * bl;
    uint64_t lo = (al * bl) >> fix::FBITS;
    return fill_fix64(hi + lo);
}

// considers overflows and rounds off fractional part
constexpr fix64 safe_mul(fix64 a, fix64 b) noexcept {
    constexpr uint64_t HALF = fix::SCALE / 2;
    int64_t av = pour_int64(a), bv = pour_int64(b);
    uint64_t ah = av >> fix::FBITS, al = av & fix::FPART;
    uint64_t bh = bv >> fix::FBITS, bl = bv & fix::FPART;
    uint64_t hi = av * bh + ah * bl, lv = al * bl;
    uint64_t lo = (lv >> fix::FBITS) + bool(lv & HALF);
    return fill_fix64(hi + lo);
}

constexpr fix64 operator/(fix64 a, int64_t n) noexcept {
    int64_t av = pour_int64(a);
    return fill_fix64(av / n);
}

// small numerator and small denominator
constexpr fix64 fast_div(int64_t a, int64_t b) noexcept {
    int64_t av = uint64_t(a) << fix::FBITS;
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
        make_fix64(+4.710874778818171503670e-7),
        make_fix64(-2.520204237306060548105e-5),
        make_fix64(+9.192602748394265802417e-4),
        make_fix64(-2.086348076335296087305e-2),
        make_fix64(+2.536695079010480136366e-1),
        make_fix64(-1.233700550136169827354e-0),
        make_fix64(+1.000000000000000000000e-0),
    };
    // using Qin J.S.'s method
    fix64 x2 = fast_mul(x, x), cosx = COEFS[0];
    cosx = fast_mul(x2, cosx) + COEFS[1];
    cosx = fast_mul(x2, cosx) + COEFS[2];
    cosx = fast_mul(x2, cosx) + COEFS[3];
    cosx = fast_mul(x2, cosx) + COEFS[4];
    cosx = fast_mul(x2, cosx) + COEFS[5];
    cosx = fast_mul(x2, cosx) + COEFS[6];
    return cosx;
}

// sin(PI/2 * x) for 0 <= x <= 1/8
constexpr fix64 sinq(fix64 x) noexcept {
    // coefficients of Taylor series
    constexpr fix64 COEFS[] = {
        make_fix64(+5.692172921967926811775e-8),
        make_fix64(-3.598843235212085340459e-6),
        make_fix64(+1.604411847873598218727e-4),
        make_fix64(-4.681754135318688100685e-3),
        make_fix64(+7.969262624616704512051e-2),
        make_fix64(-6.459640975062462536558e-1),
        make_fix64(+1.570796326794896619231e-0),
    };
    // using Qin J.S.'s method
    fix64 x2 = fast_mul(x, x), sinc = COEFS[0];
    sinc = fast_mul(x2, sinc) + COEFS[1];
    sinc = fast_mul(x2, sinc) + COEFS[2];
    sinc = fast_mul(x2, sinc) + COEFS[3];
    sinc = fast_mul(x2, sinc) + COEFS[4];
    sinc = fast_mul(x2, sinc) + COEFS[5];
    sinc = fast_mul(x2, sinc) + COEFS[6];
    // results may be incorrect via `fast_mul`
    uint64_t xv = pour_int64(x), cv = pour_int64(sinc);
    uint64_t sv = (xv * cv) >> fix::FBITS;
    return fill_fix64(sv);
}

} // namespace _tiny

// cosq(x) = cos(PI/2 * x)
constexpr fix64 cosq(fix64 x) noexcept {
    constexpr int64_t SBITS = fix::FBITS - 1;
    constexpr int64_t SPART = (1_u64 << SBITS) - 1;
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
    constexpr int64_t SBITS = fix::FBITS - 1;
    constexpr int64_t SPART = (1_u64 << SBITS) - 1;
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
