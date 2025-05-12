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
        y -= 1, m += 12;
    }
    int32_t days = (153 * m + 2) / 5 + d - 1;
    auto [c41q, c41r] = math::pydivmod<int32_t>(y, 400);
    days += (c41q - 5) * 146097;
    auto [cy4q, cy4r] = math::cdivmod<uint32_t>(c41r, 100);
    days += cy4q * 36524;
    auto [y41q, y41r] = math::cdivmod<uint32_t>(cy4r, 4);
    days += y41q * 1461 + y41r * 365;
    return days + 11017;
}

constexpr date uday_to_date(int32_t uday) noexcept {
    int32_t days = uday - 11017;
    auto [c41q, c41r] = math::pydivmod<int32_t>(days, 146097);
    int16_t y = 2000 + c41q * 400;
    if (c41r == 146096) {
        return date{int16_t(y + 400), 2, 29};
    }
    auto [cy4q, cy4r] = math::cdivmod<uint32_t>(c41r, 36524);
    auto [y41q, y41r] = math::cdivmod<uint32_t>(cy4r, 1461);
    y += cy4q * 100 + y41q * 4;
    if (y41r == 1460) {
        return date{int16_t(y + 4), 2, 29};
    }
    auto [ym5q, ym5r] = math::cdivmod<uint32_t>(y41r, 365);
    int8_t m = (5 * ym5r + 2) / 153;
    int8_t d = 1 + ym5r - (153 * m + 2) / 5;
    if ((m += 3) > 12) {
        m -= 12, y += 1;
    }
    return date{int16_t(y + ym5q), m, d};
}

enum class tzinfo: int8_t {
    west_1200,  west_1145,  west_1130,  west_1115,
    west_1100,  west_1045,  west_1030,  west_1015,
    west_1000,  west_0945,  west_0930,  west_0915,
    west_0900,  west_0845,  west_0830,  west_0815,
    west_0800,  west_0745,  west_0730,  west_0715,
    west_0700,  west_0645,  west_0630,  west_0615,
    west_0600,  west_0545,  west_0530,  west_0515,
    west_0500,  west_0445,  west_0430,  west_0415,
    west_0400,  west_0345,  west_0330,  west_0315,
    west_0300,  west_0245,  west_0230,  west_0215,
    west_0200,  west_0145,  west_0130,  west_0115,
    west_0100,  west_0045,  west_0030,  west_0015,
    utc, // Universal Time Coordinated
    east_0015,  east_0030,  east_0045,  east_0100,
    east_0115,  east_0130,  east_0145,  east_0200,
    east_0215,  east_0230,  east_0245,  east_0300,
    east_0315,  east_0330,  east_0345,  east_0400,
    east_0415,  east_0430,  east_0445,  east_0500,
    east_0515,  east_0530,  east_0545,  east_0600,
    east_0615,  east_0630,  east_0645,  east_0700,
    east_0715,  east_0730,  east_0745,  east_0800,
    east_0815,  east_0830,  east_0845,  east_0900,
    east_0915,  east_0930,  east_0945,  east_1000,
    east_1015,  east_1030,  east_1045,  east_1100,
    east_1115,  east_1130,  east_1145,  east_1200,
    east_1215,  east_1230,  east_1245,  east_1300,
    east_1315,  east_1330,  east_1345,  east_1400,
};

constexpr int64_t zone_to_offset(tzinfo zone) noexcept {
    return 900 * (int64_t(zone) - int64_t(tzinfo::utc));
}

struct dati {
    int16_t year;
    int8_t mon;
    int8_t day;
    int8_t hour;
    int8_t min;
    int8_t sec;
    tzinfo zone;
};

constexpr date dati_to_date(dati zond) noexcept {
    return date{zond.year, zond.mon, zond.day};
}

constexpr int64_t dati_to_usec(dati zond) noexcept {
    auto [y, m, d, hh, mm, ss, zz] = zond;
    int32_t uday = date_to_uday(date{y, m, d});
    int32_t dsec = int32_t(3600) * hh + 60 * mm + ss;
    int32_t zsec = zone_to_offset(zz);
    return int64_t(86400) * uday + dsec - zsec;
}

constexpr dati usec_to_dati(int64_t usec, tzinfo zone) noexcept {
    int64_t lsec = usec + zone_to_offset(zone);
    auto [uday, dsec] = math::pydivmod<int64_t>(lsec, 86400);
    auto [y, m, d] = uday_to_date(uday);
    auto [hh, hm] = math::cdivmod<uint32_t>(dsec, 3600);
    auto [mm, ss] = math::cdivmod<uint32_t>(hm, 60);
    return dati{y, m, d, int8_t(hh), int8_t(mm), int8_t(ss), zone};
}

constexpr dati zone_cast(dati zond, tzinfo zone) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_dati(usec, zone);
}

} // namespace iw17

#endif // IW_DATI_HPP