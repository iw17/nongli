#include <cstdio>

#include "nongli.hpp"

constexpr bool operator==(iw17::date a, iw17::date b) noexcept {
    return a.year == b.year && a.mon == b.mon && a.day && b.day;
}

constexpr bool operator!=(iw17::date a, iw17::date b) noexcept {
    return !(a == b);
}

constexpr bool operator==(iw17::dati a, iw17::dati b) noexcept {
    if (a.zone != b.zone || iw17::dati_to_date(a) != iw17::dati_to_date(b)) {
        return false;
    }
    return a.hour == b.hour && a.min == b.min && a.sec == b.sec;
}

constexpr bool operator!=(iw17::dati a, iw17::dati b) noexcept {
    return !(a == b);
}

constexpr bool operator==(iw17::shihou a, iw17::shihou b) noexcept {
    return a.sui == b.sui && a.jie == b.jie;
}

constexpr bool operator!=(iw17::shihou a, iw17::shihou b) noexcept {
    return !(a == b);
}

constexpr bool operator==(iw17::riqi a, iw17::riqi b) noexcept {
    return a.nian == b.nian && a.ryue == b.ryue && a.tian == b.tian;
}

constexpr bool operator!=(iw17::riqi a, iw17::riqi b) noexcept {
    return !(a == b);
}

constexpr bool operator==(iw17::bazi a, iw17::bazi b) noexcept {
    return a.zhu[0] == b.zhu[0] && a.zhu[1] == b.zhu[1]
        && a.zhu[2] == b.zhu[2] && a.zhu[3] == b.zhu[3];
}

constexpr bool operator!=(iw17::bazi a, iw17::bazi b) noexcept {
    return !(a == b);
}

int32_t test() noexcept {
    int32_t score = 0;
    constexpr int32_t N = 4;
    constexpr int64_t usecs[N] = {
        0, 1079857804, 1709136666, 1738617617,
    };
    constexpr int32_t udays[N] = {
        0, 12498, 19782, 20123,
    };
    constexpr iw17::dati zonds[N] = {
        iw17::dati{1970,  1,  1,  8,  0,  0,  32},
        iw17::dati{2004,  3, 21, 16, 30,  4,  32},
        iw17::dati{2024,  2, 29,  0, 11,  6,  32},
        iw17::dati{2025,  2,  4,  5, 20, 17,  32},
    };
    constexpr iw17::date locds[N] = {
        iw17::date{1970,  1,  1},
        iw17::date{2004,  3, 21},
        iw17::date{2024,  2, 29},
        iw17::date{2025,  2,  4},
    };
    using gz = iw17::ganzhi; // not `using enum`, compatible with C++17
    constexpr iw17::bazi bazis[N] = {
        iw17::bazi{gz::ji_you, gz::bing_zi, gz::xin_si, gz::ren_chen},
        iw17::bazi{gz::jia_shen, gz::ding_mao, gz::ji_hai, gz::ren_shen},
        iw17::bazi{gz::jia_chen, gz::bing_yin, gz::ren_xu, gz::ren_zi},
        iw17::bazi{gz::yi_si, gz::wu_yin, gz::jia_chen, gz::ding_mao},
    };
    constexpr iw17::riqi rizis[N] = {
        iw17::riqi{1969, 22, 24},
        iw17::riqi{2004,  5,  1},
        iw17::riqi{2024,  2, 20},
        iw17::riqi{2025,  2,  7},
    };
    constexpr iw17::shihou shis[N] = {
        iw17::shihou{1970, iw17::jieqi::dongzhi},
        iw17::shihou{2004, iw17::jieqi::chunfen},
        iw17::shihou{2024, iw17::jieqi::yushui},
        iw17::shihou{2025, iw17::jieqi::lichun},
    };
    constexpr iw17::dati datis[N] = {
        iw17::dati{1969, 12, 22,  8, 43, 41, 32},
        iw17::dati{2004,  3, 20, 14, 48, 38, 32},
        iw17::dati{2024,  2, 19, 12, 13, 10, 32},
        iw17::dati{2025,  2,  3, 22, 10, 26, 32},
    };
    constexpr iw17::riqi nn01s[N] = {
        iw17::riqi{1970, 22, 24},
        iw17::riqi{2005,  4,  1},
        iw17::riqi{2025,  2, 20},
        iw17::riqi{2026,  2,  7},
    };
    constexpr iw17::riqi nn19s[N] = {
        iw17::riqi{1988, 22, 24},
        iw17::riqi{2023,  5,  1},
        iw17::riqi{2043,  2, 20},
        iw17::riqi{2044,  2,  7},
    };
    for (int32_t i = 0; i < N; i++) {
        int64_t us = usecs[i];
        int32_t ud = iw17::usec_to_uday(us);
        if (udays[i] != ud) {
            return score;
        }
        score += 1;
        iw17::dati zd = iw17::usec_to_dati(us, 32);
        if (zonds[i] != zd) {
            return score;
        }
        score += 1;
        iw17::date ld = iw17::uday_to_date(ud);
        if (locds[i] != ld) {
            return score;
        }
        score += 1;
        iw17::bazi bz = iw17::usec_to_bazi(us, 119.0);
        if (bazis[i] != bz) {
            return score;
        }
        score += 1;
        iw17::riqi rz = iw17::uday_to_riqi(ud);
        if (rizis[i] != rz) {
            return score;
        }
        score += 1;
        iw17::shihou sh = iw17::usec_to_shihou(us);
        if (shis[i] != sh) {
            return score;
        }
        score += 1;
        iw17::dati dt = iw17::shihou_to_dati(sh);
        if (datis[i] != dt) {
            return score;
        }
        score += 1;
        iw17::riqi n01 = iw17::next_nian(rz, 1);
        if (nn01s[i] != n01) {
            return score;
        }
        score += 1;
        iw17::riqi n19 = iw17::next_nian(rz, 19);
        if (nn19s[i] != n19) {
            return score;
        }
        score += 1;
    }
    return score;
}

int main() {
    int32_t score = test();
    std::printf("%d scores out of %d\n", score, 9 * 4);
    return 0;
}