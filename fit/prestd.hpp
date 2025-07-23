#ifndef IW_PRESTD_HPP
#define IW_PRESTD_HPP 20250613L

#include <type_traits>

namespace iw17::prestd {

inline namespace literal {

using uintmax = unsigned long long;

#define IW_DEFINE_INT_LITERAL(suf, T)               \
constexpr T operator""_##suf(uintmax n) noexcept {  \
    return static_cast<T>(n);                       \
}

IW_DEFINE_INT_LITERAL(i16, int16_t)
IW_DEFINE_INT_LITERAL(i32, int32_t)
IW_DEFINE_INT_LITERAL(i64, int64_t)
IW_DEFINE_INT_LITERAL(u16, uint16_t)
IW_DEFINE_INT_LITERAL(u32, uint32_t)
IW_DEFINE_INT_LITERAL(u64, uint64_t)

#undef IW_DEFINE_INT_LITERAL

} // namespace literal

[[noreturn]] inline void unreachable() {
#ifdef _MSC_VER // MSVC
    __assume(false);
#else // GCC or Clang
    __builtin_unreachable();
#endif // _MSC_VER
}

template <class Out, class In>
[[nodiscard]] constexpr std::enable_if_t<
    sizeof(Out) == sizeof(In) &&
    std::is_trivially_copyable_v<Out> &&
    std::is_trivially_copyable_v<In>,
Out> bit_cast(In val) noexcept {
    return __builtin_bit_cast(Out, val);
}

namespace _sct { // struct

template <uint64_t N>
struct uint {}; // `type` not defined

#define IW_UINT_SPECIALIZATION(N, T) \
template <> struct uint<N> { using type = T; };

IW_UINT_SPECIALIZATION(1_u64, uint8_t)
IW_UINT_SPECIALIZATION(2_u64, uint16_t)
IW_UINT_SPECIALIZATION(4_u64, uint32_t)
IW_UINT_SPECIALIZATION(8_u64, uint64_t)

#undef IW_UINT_SPECIALIZATION

} // namespace _sct: struct

// only for types with size 1, 2, 4, 8
template <uint64_t N>
using uint = typename _sct::uint<N>::type;

// only for types with compact layout
template <class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_trivially_copyable_v<T>,
uint<sizeof(T)>> uint_cast(T val) noexcept {
    return bit_cast<uint<sizeof(T)>>(val);
}

} // namespace iw17::prestd

#endif // IW_PRESTD_HPP