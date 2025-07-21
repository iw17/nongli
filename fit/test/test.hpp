#ifndef IW_TEST_HPP
#define IW_TEST_HPP 20250820L

#include "../nongli.hpp"
#include "../prestd.hpp"

#define IW_ENABLE_EQUAL_BY(T, by)               \
constexpr bool operator==(T a, T b) noexcept {  \
    return by(a) == by(b);                      \
}

#define IW_ENABLE_EQUAL_BY_PAIR(T)              \
constexpr bool operator==(T a, T b) noexcept {  \
    auto [a1, a2] = a; auto [b1, b2] = b;       \
    return a1 == b1 && a2 == b2;                \
}

namespace iw17 {

IW_ENABLE_EQUAL_BY(date, prestd::uint_cast)
IW_ENABLE_EQUAL_BY(dati, prestd::uint_cast)
IW_ENABLE_EQUAL_BY(riqi, prestd::uint_cast)
IW_ENABLE_EQUAL_BY(bazi, prestd::uint_cast)

IW_ENABLE_EQUAL_BY_PAIR(shihou)

} // namespace iw17

#undef IW_ENABLE_EQUAL_BY_UINT
#undef IW_ENABLE_EQUAL_BY_PAIR

#include "suite.hpp"

#endif // IW_TEST_HPP