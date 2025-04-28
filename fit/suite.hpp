#ifndef IW_TEST_HPP
#define IW_TEST_HPP 20250628L

#include <chrono>
#include <cinttypes>
#include <cstdio>

namespace iw17 {

struct test_suite {

using str_t = const char *;
using dur_t = std::chrono::nanoseconds;

uint32_t pass;
uint32_t fail;

test_suite() noexcept: pass(0), fail(0) {
    char buf[32] = "";
    std::time_t now = std::time(nullptr);
    std::tm *ptm = std::localtime(&now);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptm);
    std::printf("Test suite initialized at %s\n\n", buf);
}

template <class Ret, class... Args>
bool test(str_t msg, const Ret &real, Ret (*func)(Args...), Args... args) {
    constexpr str_t FMT[] = {
        "[ ] %" PRIu32 "th test passed on %s",
        "[X] %" PRIu32 "th test failed on %s",
        ", taking %" PRId64 " ns\n",
    };
    auto t0 = std::chrono::steady_clock::now();
    bool pass = func(args...) == real;
    auto t1 = std::chrono::steady_clock::now();
    std::FILE *fp = nullptr;
    uint32_t count = this->pass + this->fail;
    if (pass) {
        std::fprintf((fp = stdout), FMT[0], count, msg);
        this->pass += 1;
    } else {
        std::fprintf((fp = stderr), FMT[1], count, msg);
        this->fail += 1;
    }
    dur_t dur = std::chrono::duration_cast<dur_t>(t1 - t0);
    std::fprintf(fp, FMT[2], dur.count());
    return pass;
}

uint32_t complete() const noexcept {
    uint32_t total = this->pass + this->fail;
    constexpr str_t FMT[] = {
        "%" PRIu32 " tests all passed\n",
        "%" PRIu32 " tests of %" PRIu32 " failed\n",
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

#endif // IW_TEST_HPP