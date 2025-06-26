# `Nongli`: Chinese Calendar

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](README-zh.md)
[![en](https://img.shields.io/badge/lang-en-blue.svg)](README-en.md)

> In Chinese, `nong` means agriculture and `li` means calendar.

`Nongli` is a Chinese traditional calendar, depending on predicted positions of the Sun and the Moon.

[Purple Mountain Observatory, CAS](http://english.pmo.cas.cn/) is responsible for calculations and promulgations of `nongli` according to Chinese standard [GB/T 33661-2017](https://std.samr.gov.cn/gb/search/gbDetailed?id=71F772D817FDD3A7E05397BE0A0AB82A). As a lunisolar calendar, `nongli` reflects seasonality and phenology, indicates moon phases and tides, and is widely used in daily life and production activities such as agriculture, fishery, flood prevention, and sailing.

## Table of Contents

1. [Data](#data)
    1. [Environment Setup](#environment-setup)
    1. [Fetching Raw Data](#fetching-raw-data)
    1. [Rearranging Data](#rearranging-data)
    1. [Generating Data File](#generating-data-file)
1. [Fitting](#fitting)
    1. [Running Test Examples](#running-test-examples)
    1. [`Riqi`: Date in `Nongli`](#riqi-date-in-nongli)
    1. [`Shengri`: Birthday in `Nongli`](#shengri-birthday-in-nongli)
    1. [`Jieqi`: a.k.a. Solar Term](#jieqi-aka-solar-term)
    1. [`Shihou`](#shihou)
    1. [Zodiac Sign](#zodiac-sign)
    1. [`Ganzhi`](#ganzhi)
    1. [`Futian`](#futian)
    1. [`Shujiu`](#shujiu)
    1. [`Bazi`](#bazi)
        1. [`Nian Zhu`](#nian-zhu)
        1. [`Yue Zhu`](#yue-zhu)
        1. [`Ri Zhu`](#ri-zhu)
        1. [`Shi Zhu`](#shi-zhu)
1. [Explanations](#explanations)
1. [Acknowledgements](#acknowledgements)
1. [Appendices](#appendices)
    1. [24 `Jieqi`s](#24-jieqis)
    1. [12 Zodiac Signs](#12-zodiac-signs)
    1. [10 `Tiangan`s](#10-tiangans)
    1. [12 `Dizhi`s](#12-dizhis)
    1. [Vocabulary](#vocabulary)

## Data

It is assumed in this section that the user's PWD is `nongli/data`. You can skip this section if satisfied with the data exported from 1900 to 2199 in `fit/data.hpp`. Provided on [GitHub Releases](https://github.com/iw17/nongli/releases) are a few `data.hpp` files corresponding to different time intervals. If not content with them, you can refer to this section and export one.

All the Python code here can run with Python 3.11.2. I guess it should be OK with later Python. If not, welcome to create [issues](https://github.com/iw17/nongli/issues).

### Environment Setup

To set up the environment, use the following command in [venv](https://docs.python.org/3/library/venv.html):

```bash
python -m pip install -r requirements.txt
```

### Fetching Raw Data

To fetch raw data from 1900 to 2199 and export them to `data/build/raw.txt`, use the following command:

```bash
python -u spider.py -l 1900 -u 2199
```

The lower and upper time bounds above are modifiable. Note that data before -4712 A.D. (4713 B.C.) and after 9999 A.D. may be inaccurate due to the limited precision of the fitting algorithms in SXWNL.

### Rearranging Data

The ordinals of `yue`s have changed several times throughout history. For convenience, previous `nian`s and `yue`s are rearranged according to the present `nongli`.

To export rearranged info on `shuo`s and `jieqi`s into `data/build`, use the following command:

```bash
python -u split.py
```

Note that this repository is NOT applicable for **historical** calendars **actually** used in and before 240s A.D.

### Generating Data File

To export to `fit/data.hpp` fitting arguments and residuals from 1900 to 2199, use the following command:

```bash
python -u coefs.py -l 1900 -u 2199 -o ../fit
```

The lower and upper bounds above are modifiable but must be in the original data range exported by the spider script.

## Fitting

It is assumed in this section that the user's C++ compiler is Clang, GCC or MSVC and supports 64-bit integer type and C++17 or later standard. Since C++20, signed integers shall be represented in 2's complement, and their right-shifts shall be arithmetic. In fact, most of modern C++ compilers behave this way, and thus it is assumed that the user's compiler does as well, even if compiling in C++17.

All the fittings and equation of time (EoT) bias calculations perform integral and fixed-point operations to avoid floating-point arithmetics and improve performance. 

### Running Test Examples

It is assumed the user's PWD is `nongli/fit`. To run test examples, use the following commands:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
ctest
```

### `Riqi`: Date in `Nongli`

The `chuyi` (1st) of a `nongli yue` is the day containing `shuo` (the instant when the Sun's and the Moon's apparent geocentric ecliptic longitudes meet). The `yue` on which `dongzhi` falls is p11. If there are 13 `yue`s from one p11 until next, the first `yue` that does not contain a `zhongqi` (`dongzhi`, `dahan`, ... `xiaoxue`) is called a `runyue`. Each `nongli nian` starts from one p01 until next. In this repository, `nian` starts from one `chunjie` (p01-01) until next. The value of `ryue` being 2, 3, ... 24, 25 refers to p01, r01, ... p12, r12, respectively. In Chinese, p11 and p12 are also called `dongyue` and `layue`, respectively.

> 1970 `nian` corresponds to the interval from Unix timestamp 3081600 sec until 33753600 sec.

In this document, a `runyue` starts with an `r`, while a common non-`runyue` starts with a `p`.

### `Shengri`: Birthday in `Nongli`

> In Chinese, `sheng` means birth and `ri` means day.

In this repository, `shengri` occurs when both `yue` and `tian` match the birth `riqi`. If born on 30th `tian`, then the `shengri` occurs on 29th in the `nian`s whose corresponding `yue` has only 29 `tian`s. If born in a `runyue`, the `shengri` occurs in the common non-`runyue` in the `nian`s that lack a corresponding `runyue`, or in the `runyue` in the `nian`s that do have one.

> One born in 1987-p01-01 celebrates the `shengri` on p01-01 every `nian`.\
> One born in 2004-p02-30 celebrates the `shengri` of 2023 `nian` on p02-30, and of 2024 `nian` on p02-29.\
> One born in 2004-r02-16 celebrates the `shengri` of 2022 `nian` on p02-16, and of 2023 `nian` on r02-16.

### `Jieqi`: a.k.a. Solar Term

`Jieqi` is the moment when the Sun's apparent geocentric ecliptic longitude is a multiple of 15°, and also refers to the interval from one `jieqi` moment until next. Names and longitudes of 24 `jieqi`s are listed in [Appendix](#24-jieqis). In this repository, `sui` starts from one `dongzhi` (a.k.a. winter solstice) until next. The ordinal of `jieqi` being 0, 1, ... 23 refers to `dongzhi`, `xiaohan`, ... `daxue`, where those with odd ordinals are classified as `jieling` and those with even ordinals as `zhongqi`.

> 1970 `sui` corresponds to the interval from Unix timestamp -861379 sec until 30695740 sec.

### `Shihou`

> In classical Chinese, `shi` means season and `hou` means phenology.

In this repository, `shihou` is a struct consisting of `sui` and `jieqi` stated above.

### Zodiac Sign

Here zodiac sign is primarily on astrology instead of astronomy. Zodiac signs, originated from ancient Babylon, **do NOT belong to `nongli`**, but are determined from the apparent longitude of the Sun on the ecliptic. Thus, they correspond to `jieqi`s in `nongli`. The ordinal of zodiac sign being 0, 1, ... 11 refers to Aries, Taurus, ... Pisces, respectively. Names and corresponding `jieqi`s of 12 zodiac signs are listed in [Appendix](#12-zodiac-signs). Starting with Aries (`chunfen`, a.k.a. vernal equinox), each zodiac sign follows a `zhongqi` (`chunfen`, `guyu`, ... `yushui`).

### `Ganzhi`

`Ganzhi` is a combination of `tiangan` (`jia`, `yi`, ... `gui`) and `dizhi` (`zi`, `chou`, ... `hai`), occurring in pairs for 60-periodic counting. Names of 10 `tiangan`s and 12 `dizhi`s are listed in [Appendix](#10-tiangans). The ordinal of `ganzhi` being 0, 1, ... 59 refers to `jia_zi`, `yi_chou`, ... `gui_hai`, respectively.

### `Futian`

Counting from `xiazhi` day, `toufu` (or `chufu`) starts from the 3rd `geng` day, while `erfu` (or `zhongfu`) starts from the 4th `geng` day. Counting from `liqiu` day, `sanfu` (or `mofu`) starts from the 1st `geng` day. The durations of `toufu`, `erfu` and `sanfu` are 10 days, 10 or 20 days, and 10 days, respectively, totaling either 30 or 40 days.

### `Shujiu`

> In Chinese, `jiu` means 9.

Counting from `dongzhi` day, each `jiu` consists of 9 consecutive days. `Dongzhi` day brings such 9 `jiu`s, totaling 81 days.

### `Bazi`

> In Chinese, `ba` means 8.

`Shengchen bazi` is determined from one's birth date and time, and consists of 4 `zhu`s, with 2 `zi`s (for `gan` and `zhi`) in each `zhu`. Traditionally, time was measured using sundials that display real solar time (RST). The time shown on the clock (mean solar time, MST) is converted to local time based on the birthplace's longitude, and then the Equation of Time (EoT) is added to obtain RST.

#### `Nian Zhu`

Each `nian zhu` follows a `lichun`. It is important to note that `shengxiao`s (12 animals) correspond to 12 `dizhi`s, but according to Chinese tradition, a `shengxiao` is recognized from a `chunjie` (p01-01).

> `Lichun` in 1984 `sui` is followed by a `jia_zi nian`.

#### `Yue Zhu`

Each `yue zhu` follows a `jieling` (`lichun`, `jingzhe`, ... `xiaohan`).

> `Daxue` in 2023 `sui` is followed by a `jia_zi yue`.

#### `Ri Zhu`

Each `ri zhu` follows a midnight (`zi_zheng`) in local RST.

> On the 120° E longitude, Unix timestamp 1735185600 sec falls within a `jia_zi ri`.

#### `Shi Zhu`

> In classical Chinese, `shi` also means 2 hours.

Each `shi zhu` follows an odd hour (`zi_chu` 23, `chou_chu` 1, ... `hai_chu` 21) in local RST.

> On the 120° E longitude, Unix timestamp 1738598400 sec falls within a `jia_zi shi`.

## Explanations

* Raw data are fetched from [SXWNL](https://www.sxwnl.com/super/).
* Leap seconds are not taken into account. Neither is Daylight Saving Time (DST) in China from 1986 to 1991, and Chinese Standard Time is treated as equivalent to UTC+8.
* The `bazi` calculations provided here are for reference only. Small errors around boundary instants may lead to biased or incorrect calculations.
* There is no scientific evidence to support the idea that `bazi` or zodiac signs can predict one's personality or destiny. Please approach these concepts with a critical mindset; this repository does not provide any warranties regarding the accuracy or applicability of its content.
* Some websites generate repository documentations using AI. This repository is not responsible for any mistakes or errors AI brings.

## Acknowledgements

* Thanks to Mr. Xu Jianwei for having developed the website [SXWNL](https://www.sxwnl.com/super/).
* Thanks to Ms. Zhao Yuhan for providing insights of when to celebrate `shengri` on `nongli runyue`s.

## Appendices

### 24 `Jieqi`s

| Ordinal | `Jieqi` | Meaning | Ecliptic Longitude |
|:---:|:---:|:---:|:---:|
| 0 | `dongzhi` | winter solstice | 270 |
| 1 | `xiaohan` | minor cold | 285 |
| 2 | `dahan` | major cold | 300 |
| 3 | `lichun` | beginning of spring | 315 |
| 4 | `yushui` | rain water | 330 |
| 5 | `jingzhe` | awakening of insects | 345 |
| 6 | `chunfen` | spring equinox | 0 |
| 7 | `qingming` | pure brightness | 15 |
| 8 | `guyu` | grain rain | 30 |
| 9 | `lixia` | beginning of summer | 45 |
| 10 | `xiaoman` | grain buds | 60 |
| 11 | `mangzhong` | grain in ear | 75 |
| 12 | `xiazhi` | summer solstice | 90 |
| 13 | `xiaoshu` | minor heat | 105 |
| 14 | `dashu` | major heat | 120 |
| 15 | `liqiu` | beginning of autumn | 135 |
| 16 | `chushu` | end of heat | 150 |
| 17 | `bailu` | white dew | 165 |
| 18 | `qiufen` | autumn equinox | 180 |
| 19 | `hanlu` | cold dew | 195 |
| 20 | `shuangjiang` | frost's descent | 210 |
| 21 | `lidong` | beginning of winter | 225 |
| 22 | `xiaoxue` | minor snow | 240 |
| 23 | `daxue` | major snow | 255 |

### 12 Zodiac Signs

| Ordinal | Zodiac Sign | `Jieqi`s |
|:---:|:---:|:---:|
| 0 | aries | `chunfen` & `qingming` |
| 1 | taurus | `guyu` & `lixia` |
| 2 | gemini | `xiaoman` & `mangzhong` |
| 3 | cancer | `xiazhi` & `xiaoshu` |
| 4 | leo | `dashu` & `liqiu` |
| 5 | virgo | `chushu` & `bailu` |
| 6 | libra | `qiufen` & `hanlu` |
| 7 | scorpio | `shuangjiang` & `lidong` |
| 8 | sagittarius | `xiaoxue` & `daxue` |
| 9 | capricorn | `dongzhi` & `xiaohan` |
| 10 | aquarius | `dahan` & `lichun` |
| 11 | pisces | `yushui` & `jingzhe` |

Note that the astrological names above might be different from the astronomical constellation names. *Scorpio* is astronomically called *Scorpius*.

### 10 `Tiangan`s

| Ordinal | `Tiangan` | Carbon Atoms |
|:---:|:---:|:---:|
| 0 | `jia` | 1 |
| 1 | `yi` | 2 |
| 2 | `bing` | 3 |
| 3 | `ding` | 4 |
| 4 | `wu` | 5 |
| 5 | `ji` | 6 |
| 6 | `geng` | 7 |
| 7 | `xin` | 8 |
| 8 | `ren` | 9 |
| 9 | `gui` | 10 |

Note that the ordinals in this repository are 0-base for the convenience of programming. In organic chemistry of Chinese, 10 `tiangan`s refer to 1 to 10 carbon atoms respectively, like methane (`jiawan`, $\rm{CH_4}$) and butane (`dingwan`, $\rm{C_4H_{10}}$), where `wan` means alkane in Chinese.

### 12 `Dizhi`s

| Ordinal | `Dizhi` | `Shengxiao` |
|:---:|:---:|:---:|
| 0 | `zi` | rat |
| 1 | `chou` | ox |
| 2 | `yin` | tiger |
| 3 | `mao` | rabbit |
| 4 | `chen` | loong |
| 5 | `si` | snake |
| 6 | `wu` | horse |
| 7 | `wei` | sheep |
| 8 | `shen` | monkey |
| 9 | `you` | rooster |
| 10 | `xu` | dog |
| 11 | `hai` | pig |

### Vocabulary

- `cjie`: Cumulative number of `jieqi`s since `dongzhi` of 1970 `sui` (falling on 1969 A.D.).
- `cyue`: Cumulative number of `nongli yue`s since 1970-p01.
- `date`: Gregorian date (year, month, day).
- `dati`: Gregorian date and time (year, month, day, hour, minute, second, tzinfo).
- `nian`: `Nongli` year, from one `chunjie` (p01-01) until next.
- `nyue`: Cumulative number of `nongli yue`s since the `nian`'s p01, `runyue` included.
- `riqi`: `Nongli` date, (`nian`, `ryue`, `tian`).
- `ryue`: Value of `nongli yue` with `run` (leap); the lowest bit 1 for `runyue`, and the rest bits for ordinal.
- `shihou`: `sui` and `jieqi`.
- `sui`: Approximately a tropical year, from one `dongzhi` (a.k.a. winter solstice) until next.
- `uday`: Cumulative number of days since 1970-01-01 A.D.
- `usec`: Cumulative number of seconds since 1970-01-01 A.D. 00:00:00 UTC.