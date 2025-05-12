#ifndef IW_PRESTD_HPP
#define IW_PRESTD_HPP 20250613L

namespace iw17::prestd {

[[noreturn]] void unreachable() {
    #ifdef _MSC_VER // MSVC
    __assume(false);
    #else // GCC or Clang
    __builtin_unreachable();
    #endif // _MSC_VER
}

} // namespace iw17::prestd

#endif // IW_PRESTD_HPP