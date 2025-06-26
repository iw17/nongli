#ifndef IW_TEST_HPP
#define IW_TEST_HPP 20250820L

#include "../nongli.hpp"

#define IW_ENABLE_EQUAL_BY_UINT(T)              \
constexpr bool operator==(T a, T b) noexcept {  \
    auto av = prestd::uint_cast(a);             \
    auto bv = prestd::uint_cast(b);             \
    return av == bv;                            \
}

#define IW_ENABLE_EQUAL_BY_PAIR(T)              \
constexpr bool operator==(T a, T b) noexcept {  \
    auto [a1, a2] = a;                          \
    auto [b1, b2] = b;                          \
    return a1 == b1 && a2 == b2;                \
}

namespace iw17 {

IW_ENABLE_EQUAL_BY_UINT(date)
IW_ENABLE_EQUAL_BY_UINT(dati)
IW_ENABLE_EQUAL_BY_UINT(riqi)
IW_ENABLE_EQUAL_BY_UINT(bazi)

IW_ENABLE_EQUAL_BY_PAIR(shihou)

template <class Int>
IW_ENABLE_EQUAL_BY_PAIR(math::quotrem<Int>)

} // namespace iw17

#undef IW_ENABLE_EQUAL_BY_UINT
#undef IW_ENABLE_EQUAL_BY_PAIR

using iw17::operator==;

#include "suite.hpp"

#endif // IW_TEST_HPP