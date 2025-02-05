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
    constexpr int32_t N = 3;
    constexpr int64_t usecs[N] = {
        0, 1709136666, 1738617617,
    };
    constexpr int32_t udays[N] = {
        0, 19782, 20123,
    };
    constexpr iw17::dati zonds[N] = {
        iw17::dati{1970,  1,  1,  8,  0,  0,  32},
        iw17::dati{2024,  2, 29,  0, 11,  6,  32},
        iw17::dati{2025,  2,  4,  5, 20, 17,  32},
    };
    constexpr iw17::date locds[N] = {
        iw17::date{1970,  1,  1},
        iw17::date{2024,  2, 29},
        iw17::date{2025,  2,  4},
    };
    using enum iw17::ganzhi;
    constexpr iw17::bazi bazis[N] = {
        iw17::bazi{ji_you, bing_zi, xin_si, ren_chen},
        iw17::bazi{jia_chen, bing_yin, ren_xu, ren_zi},
        iw17::bazi{yi_si, wu_yin, jia_chen, ding_mao},
    };
    constexpr iw17::riqi rizis[N] = {
        iw17::riqi{1969, 22, 24},
        iw17::riqi{2024,  2, 20},
        iw17::riqi{2025,  2,  7},
    };
    constexpr iw17::shihou shis[N] = {
        iw17::shihou{1970, iw17::jieqi::dongzhi},
        iw17::shihou{2024, iw17::jieqi::yushui},
        iw17::shihou{2025, iw17::jieqi::lichun},
    };
    constexpr iw17::dati datis[N] = {
        iw17::dati{1969, 12, 22,  8, 43, 41, 32},
        iw17::dati{2024,  2, 19, 12, 13, 10, 32},
        iw17::dati{2025,  2,  3, 22, 10, 26, 32},
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
    }
    return score;
}

int main() {
    int32_t score = test();
    std::printf("%d scores out of 21\n", score);
    return 0;
}