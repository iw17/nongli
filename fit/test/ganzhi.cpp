#include "test.hpp"

using tz = iw17::tzinfo;
using tg = iw17::tiangan;
using dz = iw17::dizhi;
using gz = iw17::ganzhi;

constexpr uint64_t N = 4;

constexpr iw17::dati datis[N] = {
    iw17::dati{1970,  1,  1,  8,  0,  0, tz::east_0800},
    iw17::dati{2004,  3, 21, 16, 30,  4, tz::east_0800},
    iw17::dati{2024,  2, 29,  0, 11,  6, tz::east_0800},
    iw17::dati{2025,  2,  4,  5, 20, 17, tz::east_0800},
};
constexpr int64_t usecs[N] = {
    0, +1079857804, +1709136666, +1738617617,
};
constexpr int32_t udays[N] = {
    0, +12498, +19782, +20123,
};
constexpr int16_t suis[N] = {
    1970, 2004, 2024, 2025,
};
constexpr iw17::bazi bazis[N] = {
    iw17::bazi{gz::ji_you, gz::bing_zi, gz::xin_si, gz::ren_chen},
    iw17::bazi{gz::jia_shen, gz::ding_mao, gz::ji_hai, gz::ren_shen},
    iw17::bazi{gz::jia_chen, gz::bing_yin, gz::ren_xu, gz::ren_zi},
    iw17::bazi{gz::yi_si, gz::wu_yin, gz::jia_chen, gz::ding_mao},
};
constexpr iw17::ganzhi nzhus[N] = {
    gz::ji_you, gz::jia_shen, gz::jia_chen, gz::yi_si,
};
constexpr iw17::ganzhi rzhus[N] = {
    gz::xin_si, gz::ji_hai, gz::gui_hai, gz::jia_chen,
};
constexpr iw17::tiangan rgans[N] = {
    tg::xin, tg::ji, tg::gui, tg::jia,
};
constexpr iw17::dizhi rzhis[N] = {
    dz::si, dz::hai, dz::hai, dz::chen,
};
constexpr iw17::riqi rizis[N] = {
    iw17::riqi{1969, 22, 24},
    iw17::riqi{2004,  5,  1},
    iw17::riqi{2024,  2, 20},
    iw17::riqi{2025,  2,  7},
};
constexpr int32_t toufus[N] = {
    +199, +12619, +19919, +20289,
};
constexpr int32_t sanfus[N] = {
    +219, +12639, +19949, +20309,
};

int main() {
    iw17::test_suite suite;
    for (uint64_t i = 0; i < N; i++) {
        suite.test("ganzhi_to_tiangan",
            rgans[i], iw17::ganzhi_to_tiangan, rzhus[i]
        );
        suite.test("ganzhi_to_dizhi",
            rzhis[i], iw17::ganzhi_to_dizhi, rzhus[i]
        );
        suite.test("make_ganzhi",
            rzhus[i], iw17::make_ganzhi, rgans[i], rzhis[i]
        );
        suite.test("nian_to_ganzhi",
            nzhus[i], iw17::nian_to_ganzhi, rizis[i].nian
        );
        suite.test("uday_to_ganzhi",
            rzhus[i], iw17::uday_to_ganzhi, udays[i]
        );
        suite.test("riqi_to_ganzhi",
            rzhus[i], iw17::riqi_to_ganzhi, rizis[i]
        );
        suite.test("ganzhi_to_riqi",
            rizis[i], iw17::ganzhi_to_riqi, rizis[i], rzhus[i]
        );
        suite.test("sui_to_toufu",
            toufus[i], iw17::sui_to_toufu, suis[i]
        );
        suite.test("sui_to_sanfu",
            sanfus[i], iw17::sui_to_sanfu, suis[i]
        );
        suite.test("usec_to_bazi",
            bazis[i], iw17::usec_to_bazi, usecs[i], 119.0
        );
        suite.test("dati_to_bazi",
            bazis[i], iw17::dati_to_bazi, datis[i], 119.0
        );
    }
    return suite.complete();
}