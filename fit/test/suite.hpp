#ifndef IW_SUITE_HPP
#define IW_SUITE_HPP 20250628L

#include <chrono>
#include <cinttypes>
#include <cstdio>

namespace iw17 {

constexpr const char *ordinal_suffix(uint32_t n) noexcept {
    constexpr const char *SUFFIXES[] = {
        "th", "st", "nd", "rd",
    };
    n %= 100;
    if (n >= 4 && n <= 20) {
        return SUFFIXES[0];
    }
    n %= 10;
    return SUFFIXES[(n >= 1 && n <= 3) ? n : 0];
}

struct test_suite {

using str_t = const char *;
using dur_t = std::chrono::nanoseconds;

uint32_t pass;
uint32_t fail;

test_suite() noexcept: pass(0), fail(0) {
    char buf[32] = "";
    std::time_t now = std::time(nullptr);
#if _MSC_VER
    std::tm stm, *ptm = &stm;
    localtime_s(ptm, &now);
#else // _MSC_VER
    std::tm *ptm = std::localtime(&now);
#endif // _MSC_VER
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptm);
    std::printf("Test suite initialized at %s\n", buf);
}

template <class Ret, class... Args>
bool test(str_t msg, const Ret &real, Ret (*func)(Args...), Args... args) {
    constexpr str_t FMT[] = {
        "[ ] %" PRIu32 "%s item passed on %s",
        "[X] %" PRIu32 "%s item failed on %s",
        ", taking %" PRId64 " ns\n",
    };
    auto t0 = std::chrono::steady_clock::now();
    bool passed = func(args...) == real;
    auto t1 = std::chrono::steady_clock::now();
    std::FILE *fp = nullptr;
    uint32_t count = this->pass + this->fail + 1;
    const char *suf = ordinal_suffix(count);
    if (passed) {
        std::fprintf((fp = stdout), FMT[0], count, suf, msg);
        this->pass += 1;
    } else {
        std::fprintf((fp = stderr), FMT[1], count, suf, msg);
        this->fail += 1;
    }
    dur_t dur = std::chrono::duration_cast<dur_t>(t1 - t0);
    std::fprintf(fp, FMT[2], dur.count());
    return passed;
}

uint32_t complete() const noexcept {
    uint32_t total = this->pass + this->fail;
    constexpr str_t FMT[] = {
        "%" PRIu32 " items all passed\n",
        "%" PRIu32 " items of %" PRIu32 " failed\n",
    };
    if (this->fail == 0) {
        std::printf(FMT[0], total);
    } else {
        std::printf(FMT[1], this->fail, total);
    }
    return this->fail;
}

}; // struct test_suite

} // namespace iw17

#endif // IW_SUITE_HPP