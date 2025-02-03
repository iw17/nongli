#ifndef IW_DATI_HPP
#define IW_DATI_HPP 20250206L

#include "math.hpp"

namespace iw17 {

struct date {
    int16_t year;
    int8_t mon;
    int8_t day;
};

constexpr int32_t date_to_uday(date locd) noexcept {
    int16_t y = locd.year;
    int8_t m = locd.mon, d = locd.day;
    if ((m -= 3) < 0) {
        y -= 1, m += 12;
    }
    int32_t days = (153 * m + 2) / 5 + d - 1;
    if (y >= 1600) {
        auto c41 = pydivmod<int32_t>(y, 400);
        days += (c41.quot - 4) * 146097;
        auto cy4 = cdivmod<int32_t>(c41.rem, 100);
        days += cy4.quot * 36524;
        auto y41 = cdivmod<int32_t>(cy4.rem, 4);
        days += y41.quot * 1461 + y41.rem * 365;
    } else {
        auto y41 = cdivmod<int32_t>(y - 1600, 4);
        days += y41.quot * 1461 + y41.rem * 365;
        days += 10 * (days < -6347);
    }
    return days - 135080;
}

constexpr date uday_to_date(int32_t uday) noexcept {
    int32_t days = uday + 135080;
    int16_t y = 1600;
    quotrem<int32_t> y41 = {0, 0};
    if (days >= 0) {
        auto c41 = pydivmod<int32_t>(days, 146097);
        if (c41.rem == 146096) {
            y += (c41.quot + 1) * 400;
            return date{y, 2, 29};
        }
        y += c41.quot * 400;
        auto cy4 = cdivmod<int32_t>(c41.rem, 36524);
        y += cy4.quot * 100;
        y41 = cdivmod<int32_t>(cy4.rem, 1461);
    } else {
        days -= 10 * (days < -6347);
        y41 = cdivmod<int32_t>(days, 1461);
    }
    if (y41.rem == 1460) {
        y += (y41.quot + 1) * 4;
        return date{y, 2, 29};
    }
    y += y41.quot * 4;
    auto ym5 = cdivmod<int32_t>(y41.rem, 365);
    y += ym5.quot;
    int8_t m = (5 * ym5.rem + 2) / 153;
    int8_t d = 1 + ym5.rem - (153 * m + 2) / 5;
    if ((m += 3) > 12) {
        m -= 12, y += 1;
    }
    return date{y, m, d};
}

struct dati {
    int16_t year;
    int8_t mon;
    int8_t day;
    int8_t hour;
    int8_t min;
    int8_t sec;
    int8_t zone;
};

constexpr date dati_to_date(dati zond) noexcept {
    return date{zond.year, zond.mon, zond.day};
}

constexpr int64_t dati_to_usec(dati zond) noexcept {
    date te = dati_to_date(zond);
    int32_t uday = date_to_uday(te);
    int32_t dsec = int32_t(3600) * zond.hour + 60 * zond.min + zond.sec;
    return int64_t(86400) * uday + dsec - int64_t(900) * zond.zone;
}

constexpr dati usec_to_dati(int64_t usec, int8_t zone) noexcept {
    int64_t lsec = usec + int64_t(900) * zone;
    quotrem<int64_t> dh = pydivmod<int64_t>(lsec, 86400);
    date locd = uday_to_date(dh.quot);
    quotrem<int32_t> hm = cdivmod<int32_t>(dh.rem, 3600);
    quotrem<int32_t> ms = cdivmod<int32_t>(ms.rem, 60);
    int8_t hour = hm.quot, min = ms.quot, sec = ms.rem;
    return dati{locd.year, locd.mon, locd.day, hour, min, sec, zone};
}

constexpr dati zone_cast(dati zodt, int8_t zone) noexcept {
    int64_t usec = dati_to_usec(zodt);
    return usec_to_dati(usec, zone);
}

} // namespace iw17

#endif // IW_DATI_HPP