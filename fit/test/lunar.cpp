#include "test.hpp"

constexpr uint64_t N = 6;

constexpr iw17::riqi rizis[N] = {
    iw17::riqi{1911, 22, 13},
    iw17::riqi{1963,  9,  7},
    iw17::riqi{1984, 24, 30},
    iw17::riqi{2006, 15, 22},
    iw17::riqi{2024, 16,  5},
    iw17::riqi{2048, 16, 30},
};
constexpr iw17::date dates[N] = {
    iw17::date{1912,  1,  1},
    iw17::date{1963,  5, 29},
    iw17::date{1985,  2, 19},
    iw17::date{2006,  9, 14},
    iw17::date{2024,  9,  7},
    iw17::date{2048, 10,  7},
};
constexpr int8_t runs[N] = {
    6, 4, 10, 7, 13, 13,
};
constexpr int8_t nyues[N] = {
    11, 4, 12, 7, 7, 7,
};
constexpr int8_t ndays[] = {
    30, 29, 30, 29, 30, 30,
};
constexpr int32_t cy01s[N] = {
    -730, -87, +173, +445, +668, +965,
};
constexpr int32_t cyues[N] = {
    -719, -83, +185, +452, +675, +972,
};
constexpr int32_t ud01s[N] = {
    -21197, -2415, +5499, +13384, +19969, +28740,
};
constexpr int32_t udays[N] = {
    -21185, -2409, +5528, +13405, +19973, +28769,
};
constexpr iw17::riqi nn19s[N] = {
    iw17::riqi{1930, 22, 13},
    iw17::riqi{1982,  9,  7},
    iw17::riqi{2003, 24, 30},
    iw17::riqi{2025, 14, 22},
    iw17::riqi{2043, 16,  5},
    iw17::riqi{2067, 16, 29},
};
constexpr iw17::riqi ny42s[N] = {
    iw17::riqi{1915,  8, 13},
    iw17::riqi{1966, 18,  7},
    iw17::riqi{1988, 10, 30},
    iw17::riqi{2009, 24, 22},
    iw17::riqi{2028,  2,  5},
    iw17::riqi{2052,  2, 29},
};
constexpr iw17::riqi nt83s[N] = {
    iw17::riqi{1912,  4,  6},
    iw17::riqi{1963, 14,  2},
    iw17::riqi{1985,  6, 24},
    iw17::riqi{2006, 20, 16},
    iw17::riqi{2024, 20, 29},
    iw17::riqi{2048, 22, 25},
};

int main() {
    using namespace iw17::prestd::literal;
    iw17::test_suite suite;
    for (uint64_t i = 0; i < N; i++) {
        suite.test("ryue_to_nyue",
            nyues[i], iw17::ryue_to_nyue, rizis[i].ryue, runs[i]
        );
        suite.test("nyue_to_ryue",
            rizis[i].ryue, iw17::nyue_to_ryue, nyues[i], runs[i]
        );
        suite.test("nian_to_run",
            runs[i], iw17::nian_to_run, rizis[i].nian
        );
        suite.test("nian_to_cyue",
            cy01s[i], iw17::nian_to_cyue, rizis[i].nian
        );
        suite.test("cyue_to_uday",
            ud01s[i], iw17::cyue_to_uday, cyues[i]
        );
        suite.test("days_in_cyue",
            ndays[i], iw17::days_in_cyue, cyues[i]
        );
        suite.test("uday_to_cyue",
            cyues[i], iw17::uday_to_cyue, udays[i]
        );
        suite.test("cyue_to_nian",
            rizis[i].nian, iw17::cyue_to_nian, cyues[i]
        );
        suite.test("uday_to_riqi",
            rizis[i], iw17::uday_to_riqi, udays[i]
        );
        suite.test("date_to_riqi",
            rizis[i], iw17::date_to_riqi, dates[i]
        );
        suite.test("riqi_to_uday",
            udays[i], iw17::riqi_to_uday, rizis[i]
        );
        suite.test("riqi_to_date",
            dates[i], iw17::riqi_to_date, rizis[i]
        );
        suite.test("next_nian",
            nn19s[i], iw17::next_nian, rizis[i], 19_i16
        );
        suite.test("next_cyue",
            ny42s[i], iw17::next_cyue, rizis[i], 42_i32
        );
        suite.test("next_tian",
            nt83s[i], iw17::next_tian, rizis[i], 83_i32
        );
    }
    return suite.complete();
}