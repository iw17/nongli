#ifndef IW_NONGLI_HPP
#define IW_NONGLI_HPP 20250203L

#include <cstdio>

#include "data.hpp"
#include "dati.hpp"
#include "math.hpp"

namespace iw17 {

constexpr int64_t uday_to_usec(int32_t uday) noexcept {
    return int64_t(86400) * uday - 28800;
}

constexpr int32_t usec_to_uday(int64_t usec) noexcept {
    return math::pydiv<int64_t>(usec + 28800, 86400);
}

struct riqi { // `nian`, `ryue`, `tian`
    int16_t nian;
    int8_t ryue;
    int8_t tian;
};

constexpr int8_t ryue_to_nyue(int8_t ryue, int8_t run) noexcept {
    return ryue / 2 - (2 * run >= ryue);
}

constexpr int8_t nyue_to_ryue(int8_t nyue, int8_t run) noexcept {
    return (nyue + (nyue < run)) * 2 + (nyue == run);
}

constexpr int8_t nian_to_run(int16_t nian) noexcept {
    nian -= _data::NIAN_MIN;
    nian = math::clip<int16_t>(nian, 0, _data::NIAN_NUM);
    uint8_t byte = _data::NR_RUNS[nian / 2];
    int8_t run = (byte >> 4 * (nian % 2)) & 0b1111;
    return run;
}

namespace _fit { // NY: nian_to_cyue

constexpr int32_t ny_pred(int16_t nian) noexcept {
    int64_t plin = _data::NY_COEF[0] * nian + _data::NY_COEF[1];
    int64_t bfit = _data::NY_COEF[2] * nian + _data::NY_COEF[3];
    return plin + (bfit >> _data::NY_BITS);
}

constexpr int32_t ny_resy(int16_t nian) noexcept {
    nian -= _data::NIAN_MIN;
    nian = math::clip<int16_t>(nian, 0, _data::NIAN_NUM);
    auto [isub, ibit] = math::cdivmod<int32_t>(nian, 8);
    return (_data::NY_RESY[isub] >> ibit) & 1;
}

} // namespace _fit

constexpr int32_t nian_to_cyue(int16_t nian) noexcept {
    int32_t pred = _fit::ny_pred(nian);
    int32_t resy = _fit::ny_resy(nian);
    return pred + resy;
}

namespace _fit { // YD: cyue_to_uday

constexpr int32_t yd_pred(int32_t cyue) noexcept {
    int64_t plin = _data::YD_COEF[0] * cyue + _data::YD_COEF[1];
    int64_t bfit = _data::YD_COEF[2] * cyue + _data::YD_COEF[3];
    return plin + (bfit >> _data::YD_BITS);
}

constexpr int32_t yd_resd(int32_t cyue) noexcept {
    cyue -= _data::CYUE_MIN;
    cyue = math::clip<int32_t>(cyue, 0, _data::CYUE_NUM);
    constexpr int32_t SIZE = 4 * sizeof(_data::YD_RESD_0);
    auto [iarr, iloc] = math::cdivmod<int32_t>(cyue, SIZE);
    const uint8_t *arrd = _data::YD_ARRD[iloc];
    auto [isub, ibit] = math::cdivmod<int32_t>(iloc, 4);
    return (arrd[isub] >> 2 * ibit) & 0b0011;
}

} // namespace _fit

constexpr int32_t cyue_to_uday(int32_t cyue) noexcept {
    int32_t pred = _fit::yd_pred(cyue);
    int32_t resd = _fit::yd_resd(cyue);
    return pred + resd;
}

constexpr int32_t uday_to_cyue(int32_t uday) noexcept {
    int32_t bfit = _data::DY_COEF[1] * uday + _data::DY_COEF[2];
    int32_t pred = _data::DY_COEF[0] + (bfit >> _data::DY_BITS);
    int32_t pday = cyue_to_uday(pred);
    return pred - (uday < pday);
}

constexpr int16_t cyue_to_nian(int32_t cyue) noexcept {
    int32_t bfit = _data::YN_COEF[1] * cyue + _data::YN_COEF[2];
    int32_t pred = _data::YN_COEF[0] + (bfit >> _data::YN_BITS);
    int32_t pyue = nian_to_cyue(pred);
    return pred - int32_t(cyue < pyue);
}

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

constexpr riqi date_to_riqi(date locd) noexcept {
    int32_t uday = date_to_uday(locd);
    return uday_to_riqi(uday);
}

constexpr int32_t riqi_to_uday(riqi rizi) noexcept {
    int16_t nian = rizi.nian;
    int8_t ryue = rizi.ryue, tian = rizi.tian;
    int8_t run = nian_to_run(nian);
    int8_t nyue = ryue_to_nyue(ryue, run);
    int32_t cy01 = nian_to_cyue(nian);
    int32_t cyue = cy01 + nyue;
    int32_t ud01 = cyue_to_uday(cyue);
    return ud01 + tian - 1;
}

constexpr date riqi_to_date(riqi rizi) noexcept {
    int32_t uday = riqi_to_uday(rizi);
    return uday_to_date(uday);
}

constexpr riqi next_nian(riqi rizi, int16_t step) noexcept {
    if (step == 0) {
        return rizi;
    }
    int16_t nian = rizi.nian + step;
    int8_t ryue = rizi.ryue, tian = rizi.tian;
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
    int16_t nian = rizi.nian;
    int8_t ryue = rizi.ryue, tian = rizi.tian;
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

struct shihou { // `sui`, `jie`
    int16_t sui;
    jieqi jie;
};

constexpr int32_t shihou_to_cjie(shihou shi) noexcept {
    return 24 * int32_t(shi.sui - 1970) + int32_t(shi.jie);
}

constexpr shihou cjie_to_shihou(int32_t cjie) noexcept {
    auto [sui, jie] = math::pydivmod<int32_t>(cjie, 24);
    return shihou{int16_t(sui + 1970), jieqi(jie)};
}

namespace _fit { // SS: shihou_to_usec

constexpr int64_t ss_pred(shihou shi) noexcept {
    int64_t sui = shi.sui;
    int64_t plin = _data::SS_CLIN[0] * sui + _data::SS_CLIN[1];
    const auto &coef = _data::SS_COEF[int8_t(shi.jie)];
    int64_t pfit = coef[0];
    // about 1/3 faster than for-cycle
    pfit = (pfit * sui >> _data::SS_BITS) + coef[1];
    pfit = (pfit * sui >> _data::SS_BITS) + coef[2];
    pfit = (pfit * sui >> _data::SS_BITS) + coef[3];
    pfit = (pfit * sui >> _data::SS_BITS) + coef[4];
    pfit = (pfit * sui >> _data::SS_BITS) + coef[5];
    pfit = (pfit * sui >> _data::SS_BITS) + coef[6];
    return plin + pfit;
}

constexpr int64_t ss_ress(int32_t cjie) noexcept {
    cjie -= _data::CJIE_MIN;
    cjie = math::clip<int32_t>(cjie, 0, _data::CJIE_NUM);
    constexpr int32_t SIZE = sizeof(_data::SS_RESS_0) / 3 * 2;
    auto [iarr, iloc] = math::cdivmod<int32_t>(cjie, SIZE);
    const uint8_t *arrs = _data::SS_ARRS[iarr];
    int32_t isub = iloc * 3 / 2;
    //// 0x12, 0x34, 0x56 -> 0x412, 0x563
    uint32_t pair = (arrs[isub + 1] << 8) | arrs[isub];
    return (iloc & 1) ? (pair >> 4) : (pair & 0x0fff);
}

} // namespace _fit

constexpr int64_t shihou_to_usec(shihou shi) noexcept {
    int64_t pred = _fit::ss_pred(shi);
    int32_t cjie = shihou_to_cjie(shi);
    int64_t ress = _fit::ss_ress(cjie);
    return pred + ress;
}

constexpr dati shihou_to_dati(shihou shi) noexcept {
    int64_t usec = shihou_to_usec(shi);
    return usec_to_dati(usec, 32);
}

constexpr int64_t cjie_to_usec(int32_t cjie) noexcept {
    int64_t ress = _fit::ss_ress(cjie);
    shihou shi = cjie_to_shihou(cjie);
    int64_t pred = _fit::ss_pred(shi);
    return pred + ress;
}

constexpr dati cjie_to_dati(int32_t cjie) noexcept {
    int64_t usec = cjie_to_usec(cjie);
    return usec_to_dati(usec, 32);
}

constexpr int32_t usec_to_cjie(int64_t usec) noexcept {
    int32_t pbit = _data::SJ_COEF[1] * usec >> _data::SJ_BITS;
    int32_t plin = (pbit + _data::SJ_COEF[2]) >> _data::SJ_BITS;
    int32_t pred = plin + _data::SJ_COEF[0];
    shihou pshi = cjie_to_shihou(pred);
    int64_t psec = shihou_to_usec(pshi);
    return pred - (usec < psec);
}

constexpr int32_t dati_to_cjie(dati zond) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_cjie(usec);
}

constexpr shihou usec_to_shihou(int64_t usec) noexcept {
    int32_t cjie = usec_to_cjie(usec);
    return cjie_to_shihou(cjie);
}

constexpr shihou dati_to_shihou(dati zond) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_shihou(usec);
}

enum class zodiac: int8_t { // NOT part of `nongli`
    aries,      taurus,     gemini,
    cancer,     leo,        virgo,
    libra,      scorpio,    sagittarius,
    capricorn,  aquarius,   pisces,
};

constexpr zodiac jieqi_to_zodiac(jieqi jie) noexcept {
    int8_t ijie = math::clip<int8_t>(int8_t(jie), 0, 23);
    int8_t izod = (ijie - 6 + 24 * (ijie < 6)) / 2;
    return zodiac(izod);
}

constexpr zodiac usec_to_zodiac(int64_t usec) noexcept {
    shihou shi = usec_to_shihou(usec);
    return jieqi_to_zodiac(shi.jie);
}

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

enum class ganzhi: int8_t { // 10 `tiangan`s, 12 `dizhi`s
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

constexpr ganzhi nian_to_ganzhi(int16_t nian) noexcept {
    int16_t nord = math::pymod<int16_t>(nian - 4, 60);
    return ganzhi(nord);
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

constexpr int32_t sui_to_toufu(int16_t sui) noexcept {
    int64_t usxz = shihou_to_usec({sui, jieqi::xiazhi});
    int32_t udxz = usec_to_uday(usxz);
    tiangan tgxz = ganzhi_to_tiangan(bday_to_ganzhi(udxz));
    int8_t diff = int8_t(tiangan::geng) - int8_t(tgxz);
    int8_t dtxz = math::pymod<int8_t>(diff, 10);
    return udxz + dtxz + 20;
}

constexpr int32_t sui_to_sanfu(int16_t sui) noexcept {
    int64_t uslq = shihou_to_usec({sui, jieqi::liqiu});
    int32_t udlq = usec_to_uday(uslq);
    tiangan tglq = ganzhi_to_tiangan(bday_to_ganzhi(udlq));
    int8_t diff = int8_t(tiangan::geng) - int8_t(tglq);
    int8_t dtlq = math::pymod<int8_t>(diff, 10);
    return udlq + dtlq;
}

struct bazi { // 4 `zhu`s: `nian`, `yue`, `ri`, `shi`
    ganzhi zhu[4];
};

namespace _rst { // real solar time: longitude bias and EoT

constexpr double bias_lon(double lon = 120.0) noexcept {
    return 240.0 * lon;
}

constexpr double bias_eot(int64_t usec, int32_t cjie) noexcept {
    double ucen = (usec - 9.46728e8) / 3.15576e9;
    double oecc = 1.67086e-2 + ucen * -1.26e-7;
    double mano = 3.918888e-3 + usec * 3.168725186e-8;
    double s1ma = math::csin(mano);
    double s2ma = math::csin(2.0 * mano);
    double hecc = oecc * (-2.0 * s1ma + oecc * -1.25 * s2ma);
    double eobl = 6.51092e-2 + ucen * 3.61e-5;
    double tah2 = 2.0 / (1.0 + math::ccos_in45d(eobl)) - 1.0;
    int32_t ljie = math::pymod<int32_t>(cjie, 24);
    int64_t last = cjie_to_usec(cjie);
    int64_t next = cjie_to_usec(cjie + 1);
    double past = double(usec - last) / (next - last);
    double tlon = (past + ljie) / 24.0;
    double s2tl = math::csin(2.0 * tlon);
    double s4tl = math::csin(4.0 * tlon);
    double hobl = tah2 * (-s2tl + tah2 * 0.5 * s4tl);
    return 13750.98708313975701043 * (hecc + hobl);
}

} // namespace _rst: real solar time

constexpr bazi usec_to_bazi(int64_t usec, double lon) noexcept {
    int32_t cjie = usec_to_cjie(usec);
    double bias_lon = _rst::bias_lon(lon);
    double bias_eot = _rst::bias_eot(usec, cjie);
    double brst = bias_lon + bias_eot;
    int64_t rsec = usec + brst + (brst < 0.0 ? -0.5 : 0.5);
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

constexpr bazi dati_to_bazi(dati zond, double lon) noexcept {
    int64_t usec = dati_to_usec(zond);
    return usec_to_bazi(usec, lon);
}

} // namespace iw17

#endif // IW_NONGLI_HPP
