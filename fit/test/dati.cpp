#include "test.hpp"

using tz = iw17::tzinfo;

constexpr uint64_t N = 6;

constexpr iw17::dati datis[N] = {
    iw17::dati{1931,  9, 18, 22, 20,  0, tz::east_0800},
    iw17::dati{1976,  7, 28,  3, 42, 53, tz::east_0800},
    iw17::dati{2001,  9, 11,  8, 46, 40, tz::west_0400},
    iw17::dati{2008,  5, 12, 14, 28,  1, tz::east_0800},
    iw17::dati{2014,  3,  8,  1, 19, 29, tz::east_0800},
    iw17::dati{2022,  3, 21, 14, 23,  0, tz::east_0800},
};
constexpr int64_t usecs[N] = {
    -1208252400, + 207344573, +1000212400,
    +1210573681, +1394212769, +1647843780,
};
constexpr iw17::dati dutcs[N] = {
    iw17::dati{1931,  9, 18, 14, 20,  0, tz::utc},
    iw17::dati{1976,  7, 27, 19, 42, 53, tz::utc},
    iw17::dati{2001,  9, 11, 12, 46, 40, tz::utc},
    iw17::dati{2008,  5, 12,  6, 28,  1, tz::utc},
    iw17::dati{2014,  3,  7, 17, 19, 29, tz::utc},
    iw17::dati{2022,  3, 21,  6, 23,  0, tz::utc},
};
constexpr iw17::date dates[N] = {
    iw17::date{1931,  9, 18},
    iw17::date{1976,  7, 28},
    iw17::date{2001,  9, 11},
    iw17::date{2008,  5, 12},
    iw17::date{2014,  3,  8},
    iw17::date{2022,  3, 21},
};
constexpr int32_t udays[N] = {
    -13985, + 2400, +11576, +14011, +16137, +19072,
};

int main() {
    iw17::test_suite suite;
    for (uint64_t i = 0; i < N; i++) {
        suite.test("dati_to_usec",
            usecs[i], iw17::dati_to_usec, datis[i]
        );
        suite.test("usec_to_dati",
            dutcs[i], iw17::usec_to_dati, usecs[i], tz::utc
        );
        suite.test("zone_cast",
            dutcs[i], iw17::zone_cast, datis[i], tz::utc
        );
        suite.test("dati_to_date",
            dates[i], iw17::dati_to_date, datis[i]
        );
        suite.test("date_to_uday",
            udays[i], iw17::date_to_uday, dates[i]
        );
        suite.test("uday_to_date",
            dates[i], iw17::uday_to_date, udays[i]
        );
        suite.test("usec_to_uday",
            udays[i], iw17::usec_to_uday, usecs[i]
        );
    }
    return suite.complete();
}