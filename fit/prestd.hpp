#ifndef IW_PRESTD_HPP
#define IW_PRESTD_HPP 20250613L

#include <type_traits>

namespace iw17::prestd {

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

[[noreturn]] void unreachable() {
#ifdef _MSC_VER // MSVC
    __assume(false);
#else // GCC or Clang
    __builtin_unreachable();
#endif // _MSC_VER
}

template <class Dest, class Src>
[[nodiscard]] constexpr std::enable_if_t<
    sizeof(Dest) == sizeof(Src) &&
    std::is_trivially_copyable_v<Dest> &&
    std::is_trivially_copyable_v<Src>,
Dest> bit_cast(Src val) noexcept {
    return __builtin_bit_cast(Dest, val);
}

namespace _sct { // struct

template <uint64_t N>
struct uint; // incomplete type

template <>
struct uint<1_u64> {
    using type = uint8_t;
};

template <>
struct uint<2_u64> {
    using type = uint16_t;
};

template <>
struct uint<4_u64> {
    using type = uint32_t;
};

template <>
struct uint<8_u64> {
    using type = uint64_t;
};

} // namespace _sct

// only for types with size 1, 2, 4, 8
template <uint64_t N>
using uint = typename _sct::uint<N>::type;

// only for types with compact layout
template <class Src>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_trivially_copyable_v<Src>,
uint<sizeof(Src)>> uint_cast(Src val) noexcept {
    return bit_cast<uint<sizeof(Src)>>(val);
}

} // namespace iw17::prestd

#endif // IW_PRESTD_HPP