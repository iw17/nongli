#ifndef IW_NONGLI_HPP
#define IW_NONGLI_HPP 20250203L

#include <cstdint>

#include "data.hpp"
#include "dati.hpp"
#include "math.hpp"

namespace iw17 {

using _data::SUI_MIN, _data::SUI_MAX;
using _data::NIAN_MIN, _data::NIAN_MAX;
using _data::YEAR_MIN, _data::YEAR_MAX;
using _data::CJIE_MIN, _data::CJIE_MAX;
using _data::CYUE_MIN, _data::CYUE_MAX;

// 00:00:00 UTC+8 to secs since Unix Epoch
constexpr int64_t uday_to_usec(int32_t uday) noexcept {
    return int64_t(86400) * uday - 28800;
}

// secs since Unix Epoch to the UTC+8 day
constexpr int32_t usec_to_uday(int64_t usec) noexcept {
    return math::pydiv<int64_t>(usec + 28800, 86400);
}

// `nian`, `ryue`, `tian`
struct riqi {
    int16_t nian;
    int8_t ryue;
    int8_t tian;
};

constexpr int8_t ryue_to_nyue(int8_t ryue, int8_t run) noexcept {
    return (ryue >> 1) - (2 * run >= ryue);
}

constexpr int8_t nyue_to_ryue(int8_t nyue, int8_t run) noexcept {
    return (nyue + (nyue < run)) * 2 + (nyue == run);
}

// `nian` to its `runyue` ordinal, 13 for none
constexpr int8_t nian_to_run(int16_t nian) noexcept {
    auto [iloc, ibit] = math::cdivmod<uint16_t>(nian - NIAN_MIN, 2);
    return (_data::NR_RUNS[iloc] >> (4 * ibit)) & 0b1111;
}

namespace _fit { // NY: nian_to_cyue

// `nian_to_cyue` prediction
constexpr int32_t ny_pred(int16_t nian) noexcept {
    int64_t plin = _data::NY_COEF[0] * nian + _data::NY_COEF[1];
    int64_t bfit = _data::NY_COEF[2] * nian + _data::NY_COEF[3];
    return plin + (bfit >> _data::NY_BITS);
}

// `nian_to_cyue` residual in `yue`
constexpr int32_t ny_resy(int16_t nian) noexcept {
    auto [iloc, ibit] = math::cdivmod<uint32_t>(nian - NIAN_MIN, 8);
    return (_data::NY_RESY[iloc] >> ibit) & 1;
}

} // namespace _fit

// `nian` to `cyue` of its p01
constexpr int32_t nian_to_cyue(int16_t nian) noexcept {
    int32_t pred = _fit::ny_pred(nian);
    int32_t resy = _fit::ny_resy(nian);
    return pred + resy;
}

namespace _fit { // YD: cyue_to_uday

// `cyue_to_uday` prediction
constexpr int32_t yd_pred(int32_t cyue) noexcept {
    int64_t plin = _data::YD_COEF[0] * cyue + _data::YD_COEF[1];
    int64_t bfit = _data::YD_COEF[2] * cyue + _data::YD_COEF[3];
    return plin + (bfit >> _data::YD_BITS);
}

// `cyue_to_uday` residual in day
constexpr int32_t yd_resd(int32_t cyue) noexcept {
    auto [isub, ibit] = math::cdivmod<uint32_t>(cyue - CYUE_MIN, 4);
    auto [iarr, iloc] = math::cdivmod<uint32_t>(isub, _data::YD_PAGE);
    const uint8_t *arrd = _data::YD_ARRD[iarr];
    return (arrd[isub] >> (2 * ibit)) & 0b0011;
}

} // namespace _fit

// `yue`s since 1970-p01 to days since 1970-01-01
constexpr int32_t cyue_to_uday(int32_t cyue) noexcept {
    int32_t pred = _fit::yd_pred(cyue);
    int32_t resd = _fit::yd_resd(cyue);
    return pred + resd;
}

// days since 1970-01-01 to `yue`s since 1970-p01
constexpr int32_t uday_to_cyue(int32_t uday) noexcept {
    int32_t bfit = _data::DY_COEF[1] * uday + _data::DY_COEF[2];
    int32_t pred = _data::DY_COEF[0] + (bfit >> _data::DY_BITS);
    int32_t pday = cyue_to_uday(pred);
    return pred - (uday < pday);
}

// `yue`s since 1970-p01 to `nian`
constexpr int16_t cyue_to_nian(int32_t cyue) noexcept {
    int32_t bfit = _data::YN_COEF[1] * cyue + _data::YN_COEF[2];
    int32_t pred = _data::YN_COEF[0] + (bfit >> _data::YN_BITS);
    int32_t pyue = nian_to_cyue(pred);
    return pred - (cyue < pyue);
}

// days since 1970-01-01 to `nongli riqi`
constexpr riqi uday_to_riqi(int32_t uday) noexcept {
    int32_t cyue = uday_to_cyue(uday);
    int16_t nian = cyue_to_nian(cyue);
    int32_t cy01 = nian_to_cyue(nian);
    int8_t nyue = cyue - cy01;
    int8_t run = nian_to_run(nian);
    int8_t ryue = nyue_to_ryue(nyue, run);
    int32_t ud01 = cyue_to_uday(cyue);
    int8_t tian = uday - ud01 + 1;
    return riqi{nian, ryue, tian};
}

// Gregorian date to `nongli riqi`
constexpr riqi date_to_riqi(date locd) noexcept {
    int32_t uday = date_to_uday(locd);
    return uday_to_riqi(uday);
}

// `nongli riqi` to days since 1970-01-01
constexpr int32_t riqi_to_uday(riqi rizi) noexcept {
    auto [nian, ryue, tian] = rizi;
    int8_t run = nian_to_run(nian);
    int8_t nyue = ryue_to_nyue(ryue, run);
    int32_t cy01 = nian_to_cyue(nian);
    int32_t cyue = cy01 + nyue;
    int32_t ud01 = cyue_to_uday(cyue);
    return ud01 + tian - 1;
}

// `nongli riqi` to Gregorian date
constexpr date riqi_to_date(riqi rizi) noexcept {
    int32_t uday = riqi_to_uday(rizi);
    return uday_to_date(uday);
}

constexpr riqi next_nian(riqi rizi, int16_t step) noexcept {
    if (step == 0) {
        return rizi;
    }
    auto [nian, ryue, tian] = rizi;
    nian += step;
    int8_t run = nian_to_run(nian);
    if (ryue & 1 && ryue >> 1 != run) {
        ryue &= ~1;
    }
    if (tian >= 30) {
        int8_t nyue = ryue_to_nyue(ryue, run);
        int32_t cy01 = nian_to_cyue(nian);
        int32_t cyue = cy01 + nyue;
        int32_t ud01 = cyue_to_uday(cyue);
        int32_t nd01 = cyue_to_uday(cyue + 1);
        tian = nd01 - ud01;
    }
    return riqi{nian, ryue, tian};
}

constexpr riqi next_cyue(riqi rizi, int32_t step) noexcept {
    if (step == 0) {
        return rizi;
    }
    auto [nian, ryue, tian] = rizi;
    int32_t cy01 = nian_to_cyue(nian);
    int8_t run = nian_to_run(nian);
    int8_t nyue = ryue_to_nyue(ryue, run);
    int32_t cyue = cy01 + nyue + step;
    nian = cyue_to_nian(cyue);
    cy01 = nian_to_cyue(nian);
    nyue = cyue - cy01;
    run = nian_to_run(nian);
    ryue = nyue_to_ryue(nyue, run);
    if (tian >= 30) {
        int32_t ud01 = cyue_to_uday(cyue);
        int32_t nd01 = cyue_to_uday(cyue + 1);
        tian = nd01 - ud01;
    }
    return riqi{nian, ryue, tian};
}

constexpr riqi next_tian(riqi rizi, int32_t step) noexcept {
    if (step == 0) {
        return rizi;
    }
    int32_t uday = riqi_to_uday(rizi) + step;
    return uday_to_riqi(uday);
}

// a.k.a. solar terms
enum class jieqi: int8_t {
    dongzhi,    xiaohan,    dahan,
    lichun,     yushui,     jingzhe,
    chunfen,    qingming,   guyu,
    lixia,      xiaoman,    mangzhong,
    xiazhi,     xiaoshu,    dashu,
    liqiu,      chushu,     bailu,
    qiufen,     hanlu,      shuangjiang,
    lidong,     xiaoxue,    daxue,
};

// consisting of `sui` and `jieqi`
struct shihou {
    int16_t sui;
    jieqi jie;
};

// `sui` and `jie` to `jieqi`s since 1970 `dongzhi`
constexpr int32_t shihou_to_cjie(shihou shi) noexcept {
    auto [sui, jie] = shi;
    return 24 * int32_t(sui - 1970) + int32_t(jie);
}

// `jieqi`s since 1970 `dongzhi` to `sui` and `jie`
constexpr shihou cjie_to_shihou(int32_t cjie) noexcept {
    auto [sui, jie] = math::pydivmod<int32_t>(cjie, 24);
    return shihou{int16_t(sui + 1970), jieqi(jie)};
}

namespace _fit { // JS: cjie_to_usec

// `cjie_to_usec` prediction
constexpr int64_t js_pred(shihou shi) noexcept {
    auto [sui, jie] = shi;
    int64_t plin = _data::JS_CLIN[0] * sui + _data::JS_CLIN[1];
    const auto &coef = _data::JS_COEF[int8_t(jie)];
    int64_t pfit = coef[0];
    // about 1/3 faster than for-loop
    pfit = (pfit * sui >> _data::JS_BITS) + coef[1];
    pfit = (pfit * sui >> _data::JS_BITS) + coef[2];
    pfit = (pfit * sui >> _data::JS_BITS) + coef[3];
    pfit = (pfit * sui >> _data::JS_BITS) + coef[4];
    pfit = (pfit * sui >> _data::JS_BITS) + coef[5];
    pfit = (pfit * sui >> _data::JS_BITS) + coef[6];
    return plin + pfit;
}

// `cjie_to_usec` residual in sec
constexpr int64_t js_ress(int32_t cjie) noexcept {
    int32_t isub = (cjie - CJIE_MIN) * 3 / 2;
    auto [iarr, iloc] = math::cdivmod<uint32_t>(isub, _data::JS_PAGE);
    const uint8_t *arrs = _data::JS_ARRS[iarr];
    // 0x12, 0x34, 0x56 -> 0x412, 0x563
    uint32_t pair = (arrs[iloc + 1] << 8) | arrs[iloc];
    return (cjie & 1) ? (pair >> 4) : (pair & 0x0fff);
}

} // namespace _fit

// `sui` and `jie` to secs since Unix Epoch
constexpr int64_t shihou_to_usec(shihou shi) noexcept {
    int64_t pred = _fit::js_pred(shi);
    int32_t cjie = shihou_to_cjie(shi);
    int64_t ress = _fit::js_ress(cjie);
    return pred + ress;
}

// `sui` and `jie` to Gregorian datetime
constexpr dati shihou_to_dati(shihou shi) noexcept {
    int64_t usec = shihou_to_usec(shi);
    return usec_to_dati(usec, tzinfo::east_0800);
}

// `jieqi`s since 1970 `dongzhi` to secs since Unix Epoch
constexpr int64_t cjie_to_usec(int32_t cjie) noexcept {
    int64_t ress = _fit::js_ress(cjie);
    shihou shi = cjie_to_shihou(cjie);
    int64_t pred = _fit::js_pred(shi);
    return pred + ress;
}

// `jieqi`s since 1970 `dongzhi` to Gregorian datetime UTC+8
constexpr dati cjie_to_dati(int32_t cjie) noexcept {
    int64_t usec = cjie_to_usec(cjie);
    return usec_to_dati(usec, tzinfo::east_0800);
}

// secs since Unix Epoch to `jieqi`s since 1970 `dongzhi`
constexpr int32_t usec_to_cjie(int64_t usec) noexcept {
    int32_t pbit = _data::SJ_COEF[1] * usec >> _data::SJ_BITS;
    int32_t plin = (pbit + _data::SJ_COEF[2]) >> _data::SJ_BITS;
    int32_t pred = plin + _data::SJ_COEF[0];
    shihou pshi = cjie_to_shihou(pred);
    int64_t psec = shihou_to_usec(pshi);
    return pred - (usec < psec);
}

// Gregorian datetime to `jieqi`s since 1970 `dongzhi`
constexpr int32_t dati_to_cjie(dati zond) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_cjie(usec);
}

// secs since Unix Epoch to `sui` and `jie`
constexpr shihou usec_to_shihou(int64_t usec) noexcept {
    int32_t cjie = usec_to_cjie(usec);
    return cjie_to_shihou(cjie);
}

// Gregorian datetime to `sui` and `jie`
constexpr shihou dati_to_shihou(dati zond) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_shihou(usec);
}

// zodiac signs, NOT part of `nongli`
enum class zodiac: int8_t {
    aries,      taurus,     gemini,
    cancer,     leo,        virgo,
    libra,      scorpio,    sagittarius,
    capricorn,  aquarius,   pisces,
};

constexpr zodiac jieqi_to_zodiac(jieqi jie) noexcept {
    int8_t ijie = int8_t(jie);
    constexpr int8_t CF = int8_t(jieqi::chunfen);
    int8_t izod = (ijie - CF + 24 * (ijie < CF)) / 2;
    return zodiac(izod);
}

// secs since Unix Epoch to zodiac sign
constexpr zodiac usec_to_zodiac(int64_t usec) noexcept {
    shihou shi = usec_to_shihou(usec);
    return jieqi_to_zodiac(shi.jie);
}

// Gregorian datetime to zodiac sign
constexpr zodiac dati_to_zodiac(dati zond) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_zodiac(usec);
}

enum class tiangan: int8_t {
    jia,    yi,     bing,   ding,   wu,
    ji,     geng,   xin,    ren,    gui,
};

enum class dizhi: int8_t {
    zi,     chou,   yin,    mao,    chen,   si,
    wu,     wei,    shen,   you,    xu,     hai,
};

// combination of 10 `tiangan`s and 12 `dizhi`s
enum class ganzhi: int8_t {
    jia_zi,     yi_chou,    bing_yin,   ding_mao,   wu_chen,
    ji_si,      geng_wu,    xin_wei,    ren_shen,   gui_you,
    jia_xu,     yi_hai,     bing_zi,    ding_chou,  wu_yin,
    ji_mao,     geng_chen,  xin_si,     ren_wu,     gui_wei,
    jia_shen,   yi_you,     bing_xu,    ding_hai,   wu_zi,
    ji_chou,    geng_yin,   xin_mao,    ren_chen,   gui_si,
    jia_wu,     yi_wei,     bing_shen,  ding_you,   wu_xu,
    ji_hai,     geng_zi,    xin_chou,   ren_yin,    gui_mao,
    jia_chen,   yi_si,      bing_wu,    ding_wei,   wu_shen,
    ji_you,     geng_xu,    xin_hai,    ren_zi,     gui_chou,
    jia_yin,    yi_mao,     bing_chen,  ding_si,    wu_wu,
    ji_wei,     geng_shen,  xin_you,    ren_xu,     gui_hai,
};

constexpr tiangan ganzhi_to_tiangan(ganzhi zhu) noexcept {
    return tiangan(int8_t(zhu) % 10);
}

constexpr dizhi ganzhi_to_dizhi(ganzhi zhu) noexcept {
    return dizhi(int8_t(zhu) % 12);
}

constexpr ganzhi make_ganzhi(tiangan gan, dizhi zhi) noexcept {
    int8_t zord = 6 * int8_t(gan) - 5 * int8_t(zhi);
    return ganzhi(zord + 60 * (zord < 0));
}

// `nian` to `ganzhi nianzhu`
constexpr ganzhi nian_to_ganzhi(int16_t nian) noexcept {
    int16_t nord = math::pymod<int16_t>(nian - 4, 60);
    return ganzhi(nord);
}

// days since 1970-01-01 to `ganzhi rizhu`
constexpr ganzhi uday_to_ganzhi(int32_t uday) noexcept {
    int32_t dord = math::pymod<int32_t>(uday + 17, 60);
    return ganzhi(dord);
}

// `nongli riqi` to `ganzhi rizhu`
constexpr ganzhi riqi_to_ganzhi(riqi rizi) noexcept {
    int32_t uday = riqi_to_uday(rizi);
    return uday_to_ganzhi(uday);
}

// `nian`, `yue` and `ganzhi rizhu` to `nongli riqi`
constexpr riqi ganzhi_to_riqi(riqi nianyue, ganzhi tian) noexcept {
    auto [nian, ryue, _] = nianyue;
    ganzhi rz01 = riqi_to_ganzhi({nian, ryue, 0});
    int8_t diff = int8_t(tian) - int8_t(rz01);
    nianyue.tian = diff + 60 * (diff < 0);
    return nianyue;
}

constexpr ganzhi byue_to_ganzhi(int32_t byue) noexcept {
    int32_t yord = math::pymod<int32_t>(byue + 14, 60);
    return ganzhi(yord);
}

constexpr ganzhi bday_to_ganzhi(int32_t bday) noexcept {
    int32_t dord = math::pymod<int32_t>(bday + 17, 60);
    return ganzhi(dord);
}

constexpr ganzhi bshi_to_ganzhi(int64_t bshi) noexcept {
    int64_t sord = math::pymod<int64_t>(bshi + 24, 60);
    return ganzhi(sord);
}

// `sui` to start day of `toufu`
constexpr int32_t sui_to_toufu(int16_t sui) noexcept {
    int64_t usxz = shihou_to_usec({sui, jieqi::xiazhi});
    int32_t udxz = usec_to_uday(usxz);
    tiangan tgxz = ganzhi_to_tiangan(uday_to_ganzhi(udxz));
    int8_t diff = int8_t(tiangan::geng) - int8_t(tgxz);
    int8_t dtxz = math::pymod<int8_t>(diff, 10);
    return udxz + dtxz + 20;
}

// `sui` to start day of `sanfu`
constexpr int32_t sui_to_sanfu(int16_t sui) noexcept {
    int64_t uslq = shihou_to_usec({sui, jieqi::liqiu});
    int32_t udlq = usec_to_uday(uslq);
    tiangan tglq = ganzhi_to_tiangan(uday_to_ganzhi(udlq));
    int8_t diff = int8_t(tiangan::geng) - int8_t(tglq);
    int8_t dtlq = math::pymod<int8_t>(diff, 10);
    return udlq + dtlq;
}

// 4 `zhu`s: `nian`, `yue`, `ri`, `shi`
struct bazi {
    ganzhi zhu[4];
};

namespace _rst { // real solar time: longitude bias and EoT

// bias secs for longitude
constexpr math::fix64 bias_lon(math::fix64 lon) noexcept {
    return 240 * lon;
}

// bias secs for Equation of Time
constexpr math::fix64 bias_eot(int64_t usec, int32_t cjie) noexcept {
    using namespace math::literal;
    constexpr int64_t EPOCH_J2K = 946728000;
    constexpr int64_t SECS_CENTURY = 3155760000;
    // centuries (36525 days) since J2000.0 epoch
    int64_t jsec = usec - EPOCH_J2K;
    math::fix64 ucen = math::fair_div(jsec, SECS_CENTURY);
    // mean anomaly
    constexpr int64_t SECS_TWO_YEARS = 63116865;
    constexpr int64_t USEC_PERI = 946876650;
    int64_t psec = 8 * (usec - USEC_PERI);
    math::fix64 mano = math::fair_div(psec, SECS_TWO_YEARS);
    // eccentricity of the Earth's orbit
    constexpr math::fix64 ECCO_COEFS[] = {
        math::make_fix64(+1.67086e-2),
        math::make_fix64(-4.19300e-5),
        math::make_fix64(-1.26000e-7),
    };
    math::fix64 ecco = ECCO_COEFS[2];
    ecco = math::fast_mul(ecco, ucen) + ECCO_COEFS[1];
    ecco = math::fast_mul(ecco, ucen) + ECCO_COEFS[0];
    // hour angle bias from eccentricity
    math::fix64 s1ma = math::sinq(mano);
    math::fix64 s2ma = math::sinq(2 * mano);
    math::fix64 hecc = math::fast_mul(-5 * ecco, s2ma);
    hecc = math::fast_mul(ecco, (hecc >> 2) - 2 * s1ma);
    // obliquity of the ecliptic
    constexpr math::fix64 OBLE_COEFS[] = {
        math::make_fix64(+2.60437e-1),
        math::make_fix64(-1.44444e-4),
        math::make_fix64(-2.22222e-9),
        math::make_fix64(+5.55556e-9),
    };
    math::fix64 oble = OBLE_COEFS[3];
    oble = math::fast_mul(oble, ucen) + OBLE_COEFS[2];
    oble = math::fast_mul(oble, ucen) + OBLE_COEFS[1];
    oble = math::fast_mul(oble, ucen) + OBLE_COEFS[0];
    // true longitude of the Sun on the ecliptic
    constexpr int32_t CF = int32_t(jieqi::chunfen);
    int32_t ljie = math::pymod<int32_t>(cjie - CF, 24);
    int64_t last = cjie_to_usec(cjie);
    int64_t next = cjie_to_usec(cjie + 1);
    int64_t past = usec - last, jdur = next - last;
    int64_t pscf = ljie * jdur + past; // since `chunfen`
    math::fix64 qlon = math::fast_div(2 * pscf, 3 * jdur);
    // hour angle bias from obliquity
    math::fix64 veso = 1_fix - math::cosq(oble);
    math::fix64 s4tl = math::sinq(qlon);
    math::fix64 s2tl = math::sinq(qlon >> 1);
    math::fix64 hobl = (s2tl - (s4tl >> 1)) >> 1;
    hobl = math::fast_mul(hobl, veso) + (s2tl >> 1);
    hobl = math::fast_mul(hobl, veso);
    // time bias from hour angles
    constexpr double HAT_DBL = 13750.987083139757010431557;
    constexpr math::fix64 HAT_FIX = math::make_fix64(HAT_DBL);
    return math::fair_mul(HAT_FIX, hecc + hobl);
}

} // namespace _rst: real solar time

// secs since Unix Epoch and birthplace longitude to `bazi`
constexpr bazi usec_to_bazi(int64_t usec, double lon) noexcept {
    int32_t cjie = usec_to_cjie(usec);
    math::fix64 flon = math::make_fix64(lon);
    math::fix64 bias_lon = _rst::bias_lon(flon);
    math::fix64 bias_eot = _rst::bias_eot(usec, cjie);
    math::fix64 bias_rst = bias_lon + bias_eot;
    int64_t rsec = usec + math::safe_int(bias_rst);
    int64_t bshi = math::pydiv<int64_t>(rsec + 3600, 7200);
    int32_t bday = math::pydiv<int64_t>(rsec, 86400);
    int32_t byue = (cjie - 3) >> 1;
    int32_t bsui = 1970 + math::pydiv<int32_t>(byue, 12);
    ganzhi nzhu = nian_to_ganzhi(bsui);
    ganzhi yzhu = byue_to_ganzhi(byue);
    ganzhi rzhu = bday_to_ganzhi(bday);
    ganzhi szhu = bshi_to_ganzhi(bshi);
    return bazi{nzhu, yzhu, rzhu, szhu};
}

// Gregorian datetime and birthplace longitude to `bazi`
constexpr bazi dati_to_bazi(dati zond, double lon) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_bazi(usec, lon);
}

} // namespace iw17

#endif // IW_NONGLI_HPP
