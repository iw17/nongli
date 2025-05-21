#include "test.hpp"

using tz = iw17::tzinfo;
using jq = iw17::jieqi;
using zd = iw17::zodiac;

constexpr uint64_t N = 4;

constexpr iw17::dati datis[N] = {
    iw17::dati{1945,  8, 15, 12,  0,  0, tz::east_0900},
    iw17::dati{1970,  1,  1,  0,  0,  0, tz::utc},
    iw17::dati{1996,  7,  7,  3, 59, 59, tz::east_0800},
    iw17::dati{2024, 12,  7, 22, 56,  4, tz::east_0800},
};
constexpr int64_t usecs[N] = {
    -769381200, 0, +836683199, +1733583364,
};
constexpr iw17::shihou shis[N] = {
    iw17::shihou{1945, jq::liqiu},
    iw17::shihou{1970, jq::dongzhi},
    iw17::shihou{1996, jq::xiazhi},
    iw17::shihou{2024, jq::daxue},
};
constexpr iw17::dati jqdts[N] = {
    iw17::dati{1945,  8,  8,  6,  5,  3, tz::east_0800},
    iw17::dati{1969, 12, 22,  8, 43, 41, tz::east_0800},
    iw17::dati{1996,  6, 21, 10, 23, 44, tz::east_0800},
    iw17::dati{2024, 12,  6, 23, 17,  0, tz::east_0800},
};
constexpr int64_t jquss[N] = {
    -770003697, -861379, +835323824, +1733498220,
};
constexpr int32_t cjies[N] = {
    -585, 0, +636, +1319,
};
constexpr iw17::zodiac zods[N] = {
    zd::leo, zd::capricorn, zd::cancer, zd::sagittarius,
};

int main() {
    iw17::test_suite suite;
    for (uint64_t i = 0; i < N; i++) {
        suite.test("shihou_to_cjie",
            cjies[i], iw17::shihou_to_cjie, shis[i]
        );
        suite.test("shihou_to_dati",
            jqdts[i], iw17::shihou_to_dati, shis[i]
        );
        suite.test("cjie_to_usec",
            jquss[i], iw17::cjie_to_usec, cjies[i]
        );
        suite.test("cjie_to_dati",
            jqdts[i], iw17::cjie_to_dati, cjies[i]
        );
        suite.test("usec_to_cjie",
            cjies[i], iw17::usec_to_cjie, usecs[i]
        );
        suite.test("dati_to_cjie",
            cjies[i], iw17::dati_to_cjie, datis[i]
        );
        suite.test("usec_to_shihou",
            shis[i], iw17::usec_to_shihou, usecs[i]
        );
        suite.test("dati_to_shihou",
            shis[i], iw17::dati_to_shihou, datis[i]
        );
        suite.test("jieqi_to_zodiac",
            zods[i], iw17::jieqi_to_zodiac, shis[i].jie
        );
        suite.test("usec_to_zodiac",
            zods[i], iw17::usec_to_zodiac, usecs[i]
        );
        suite.test("dati_to_zodiac",
            zods[i], iw17::dati_to_zodiac, datis[i]
        );
    }
    return suite.complete();
}