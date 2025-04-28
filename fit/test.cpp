#include "nongli.hpp"

constexpr bool operator==(iw17::date a, iw17::date b) noexcept {
    return a.year == b.year && a.mon == b.mon && a.day == b.day;
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

#include "suite.hpp"

uint32_t test() noexcept {
    // not `using enum`, compatible with C++17
    using tz = iw17::tzinfo;
    using gz = iw17::ganzhi;
    using jq = iw17::jieqi;
    using namespace iw17::math::literal;
    //int32_t score = 0;
    constexpr int32_t N = 4;
    constexpr int64_t usecs[N] = {
        0, 1079857804, 1709136666, 1738617617,
    };
    constexpr int32_t udays[N] = {
        0, 12498, 19782, 20123,
    };
    constexpr iw17::dati zonds[N] = {
        iw17::dati{1970,  1,  1,  8,  0,  0, tz::east_0800},
        iw17::dati{2004,  3, 21, 16, 30,  4, tz::east_0800},
        iw17::dati{2024,  2, 29,  0, 11,  6, tz::east_0800},
        iw17::dati{2025,  2,  4,  5, 20, 17, tz::east_0800},
    };
    constexpr iw17::date locds[N] = {
        iw17::date{1970,  1,  1},
        iw17::date{2004,  3, 21},
        iw17::date{2024,  2, 29},
        iw17::date{2025,  2,  4},
    };
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
        iw17::shihou{1970, jq::dongzhi},
        iw17::shihou{2004, jq::chunfen},
        iw17::shihou{2024, jq::yushui},
        iw17::shihou{2025, jq::lichun},
    };
    constexpr iw17::dati datis[N] = {
        iw17::dati{1969, 12, 22,  8, 43, 41, tz::east_0800},
        iw17::dati{2004,  3, 20, 14, 48, 38, tz::east_0800},
        iw17::dati{2024,  2, 19, 12, 13, 10, tz::east_0800},
        iw17::dati{2025,  2,  3, 22, 10, 26, tz::east_0800},
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
    constexpr int32_t toufus[N] = {
        199, 12619, 19919, 20289,
    };
    constexpr int32_t sanfus[N] = {
        219, 12639, 19949, 20309,
    };
    iw17::test_suite suite;
    for (int32_t i = 0; i < N; i++) {
        suite.test("usec_to_uday",
            udays[i], iw17::usec_to_uday, usecs[i]
        );
        suite.test("usec_to_dati",
            zonds[i], iw17::usec_to_dati, usecs[i], tz::east_0800
        );
        suite.test("uday_to_date",
            locds[i], iw17::uday_to_date, udays[i]
        );
        suite.test("usec_to_bazi",
            bazis[i], iw17::usec_to_bazi, usecs[i], 119_fix
        );
        suite.test("uday_to_riqi",
            rizis[i], iw17::uday_to_riqi, udays[i]
        );
        suite.test("usec_to_shihou",
            shis[i], iw17::usec_to_shihou, usecs[i]
        );
        suite.test("shihou_to_dati",
            datis[i], iw17::shihou_to_dati, shis[i]
        );
        suite.test("next_nian_01",
            nn01s[i], iw17::next_nian, rizis[i], 1_i16
        );
        suite.test("next_nian_19",
            nn19s[i], iw17::next_nian, rizis[i], 19_i16
        );
        suite.test("sui_to_toufu",
            toufus[i], iw17::sui_to_toufu, shis[i].sui
        );
        suite.test("sui_to_sanfu",
            sanfus[i], iw17::sui_to_sanfu, shis[i].sui
        );
        std::putchar('\n');
    }
    return suite.complete();
}

int main() {
    uint32_t fail = test();
    return fail != 0;
}