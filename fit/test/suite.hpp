#ifndef IW_SUITE_HPP
#define IW_SUITE_HPP 20250628L

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

namespace iw17 {

constexpr const char *ordinal_suffix(uint64_t n) noexcept {
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

constexpr const char *plural_suffix(uint64_t n) noexcept {
    constexpr const char *SUFFIXES[] = { "s", "" };
    return SUFFIXES[n == 1];
}

struct test_suite {

using str_t = const char *;
using dur_t = std::chrono::nanoseconds;

uint64_t pass, fail;

test_suite() noexcept: pass(0), fail(0) {
    char buf[32] = "";
    std::time_t now = std::time(nullptr);
#if _MSC_VER
    std::tm stm, *ptm = &stm;
    localtime_s(ptm, &now);
#else // _MSC_VER
    std::tm *ptm = std::localtime(&now);
#endif // _MSC_VER
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", ptm);
    std::printf("Test suite initialized at %s\n", buf);
}

test_suite(const test_suite &) = delete;

template <class Ret, class... Args>
bool test(str_t msg,
    const Ret &real, Ret (*func)(Args...), Args... args
) {
    constexpr str_t FMT[] = {
        "[ ] %" PRIu64 "%s item passed on %s",
        "[X] %" PRIu64 "%s item failed on %s",
        ", taking %" PRId64 " ns\n",
    };
    auto t0 = std::chrono::steady_clock::now();
    bool passed = func(args...) == real;
    auto t1 = std::chrono::steady_clock::now();
    std::FILE *fp = nullptr;
    uint64_t count = this->pass + this->fail + 1;
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

uint64_t complete() const noexcept {
    uint64_t total = this->pass + this->fail;
    constexpr str_t FMT[] = {
        "%" PRIu64 " item%s all passed\n",
        "%" PRIu64 " item%s of %" PRIu64 " failed\n",
    };
    if (this->fail == 0) {
        const char *suf = plural_suffix(total);
        std::fprintf(stdout, FMT[0], total, suf);
    } else {
        const char *suf = plural_suffix(this->fail);
        std::fprintf(stderr, FMT[1], this->fail, suf, total);
    }
    return this->fail;
}

}; // struct test_suite

} // namespace iw17

#endif // IW_SUITE_HPP