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
    auto [y, m, d] = locd;
    if ((m -= 3) < 0) {
        y -= 1;
        m += 12;
    }
    int32_t days = (153 * m + 2) / 5 + d - 1;
    if (y >= 1600) {
        auto [c41q, c41r] = math::pydivmod<int32_t>(y, 400);
        days += (c41q - 4) * 146097;
        auto [cy4q, cy4r] = math::cdivmod<int32_t>(c41r, 100);
        days += cy4q * 36524;
        auto [y41q, y41r] = math::cdivmod<int32_t>(cy4r, 4);
        days += y41q * 1461 + y41r * 365;
    } else {
        auto [y41q, y41r] = math::cdivmod<int32_t>(y - 1600, 4);
        days += y41q * 1461 + y41r * 365;
        days += 10 * (days < -6347);
    }
    return days - 135080;
}

constexpr date uday_to_date(int32_t uday) noexcept {
    int32_t days = uday + 135080;
    int16_t y = 1600;
    math::quotrem<int32_t> y41 = {0, 0};
    if (days >= 0) {
        auto [c41q, c41r] = math::cdivmod<int32_t>(days, 146097);
        if (c41r == 146096) {
            y += (c41q + 1) * 400;
            return date{y, 2, 29};
        }
        y += c41q * 400;
        auto [cy4q, cy4r] = math::cdivmod<int32_t>(c41r, 36524);
        y += cy4q * 100;
        y41 = math::cdivmod<int32_t>(cy4r, 1461);
    } else {
        days -= 10 * (days < -6347);
        y41 = math::pydivmod<int32_t>(days, 1461);
    }
    auto [y41q, y41r] = y41;
    if (y41r == 1460) {
        y += (y41q + 1) * 4;
        return date{y, 2, 29};
    }
    y += y41q * 4;
    auto [ym5q, ym5r] = math::cdivmod<int32_t>(y41r, 365);
    y += ym5q;
    int8_t m = (5 * ym5r + 2) / 153;
    int8_t d = 1 + ym5r - (153 * m + 2) / 5;
    if ((m += 3) > 12) {
        m -= 12;
        y += 1;
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
    auto [y, m, d, hh, mm, ss, zz] = zond;
    int32_t uday = date_to_uday(date{y, m, d});
    int32_t dsec = int32_t(3600) * hh + 60 * mm + ss;
    int32_t zsec = int32_t(900) * zz;
    return int64_t(86400) * uday + dsec - zsec;
}

constexpr dati usec_to_dati(int64_t usec, int8_t zone) noexcept {
    int64_t lsec = usec + int64_t(900) * zone;
    auto [uday, dsec] = math::pydivmod<int64_t>(lsec, 86400);
    auto [y, m, d] = uday_to_date(uday);
    auto [hh, hm] = math::cdivmod<int32_t>(dsec, 3600);
    auto [mm, ss] = math::cdivmod<int32_t>(hm, 60);
    return dati{y, m, d, int8_t(hh), int8_t(mm), int8_t(ss), zone};
}

constexpr dati zone_cast(dati zond, int8_t zone) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_dati(usec, zone);
}

} // namespace iw17

#endif // IW_DATI_HPP