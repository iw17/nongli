# `Nongli`

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](README-zh.md)
[![en](https://img.shields.io/badge/lang-en-blue.svg)](README-en.md)

`Nongli` is a Chinese traditional calendar, depending on predicted positions of the Sun and the Moon.

> In Chinese, `nong` means agriculture and `li` means calendar.

[Purple Mountain Observatory, CAS](http://english.pmo.cas.cn/) is responsible for calculations and promulgations of `nongli` according to Chinese standard [GB/T 33661-2017](https://std.samr.gov.cn/gb/search/gbDetailed?id=71F772D817FDD3A7E05397BE0A0AB82A). As a lunisolar calendar, `nongli` reflects seasonality and phenology, indicates moon phases and tides, and is widely used in daily life and production activities such as agriculture, fishery, flood prevention, and sailing.

## Data

You can skip this section if satisfied with the data exported from 1900 to 2199 in `fit/data.hpp`.

### Environment Setup

To set up the environment, use the following commands:

```bash
cd data/
conda env create -f environment.yml
conda activate Nongli
```

### Fetching Raw Data

To fetch raw data and export them to `data/build/raw.txt`, use the following commands:

```bash
cd data/
python -u spider.py
```

The constants `MIN` and `MAX` in the spider script are modifiable. Note that data before -4712 A.D. (4713 B.C.) and after 9999 A.D. may be inaccurate due to the limited precision of the fitting algorithms in SXWNL.

### Rearranging Data

The ordinals of `yue`s have changed several times throughout history. For convenience, previous data are rearranged according to the present `nongli`. To export rearranged info on `shuo`s and `jieqi`s into `data/build/`, use the following command:

```bash
cd data/
python -u split.py
```

The constants `MIN` and `MAX` in the notebook code are modifiable without exceeding the original data range. Note that this repository is NOT applicable for **historical** calendars **actually** used in and before 240s A.D.

### Generating C++ Data File

Open `data/coefs.ipynb` with Jupyter Notebook and export fitting arguments and residuals to `fit/data.hpp` by running all cells.

## Fitting

All the fittings and equation of time (EoT) bias calculations perform integral and fixed-point operations to avoid floating-point arithmetic and improve performance. Since C++20, right-shifts on signed integers are supposed to be arithmetic. Most of C++ compilers perform right-shifts this way, and it is assumed that the user's compiler does as well, even if compiling in C++17.

### Running Test Examples

To run test examples, use the following commands if using GCC or Clang and Make:

```bash
cd fit/
mkdir build/ && cd build/
cmake ..
make
./test # or './test.exe' if on Windows
```

Or use the following commands if using MSVC and NMAKE:

```bash
cd fit/
mkdir build/ && cd build/
cmake ..
cmake --build . --config Release
./Release/test.exe
```

### `Riqi`: Date in `Nongli`

In this repository, `nian` starts from one `chunjie` (p01-01) until next. The value of `ryue` being 2, 3, ... 24, 25 refers to p01, r01, ... p12, r12, respectively. In Chinese, p11 and p12 are also called `dongyue` and `layue`, respectively.

> 1970 `nian` corresponds to the interval from Unix timestamp 3081600 sec until 33753600 sec.

Here a `runyue` starts with an `r`, while a common non-`runyue` starts with a `p`.

### `Shengri`: Birthday in `Nongli`

In this repository, `shengri` occurs when both `yue` and `tian` match the birth `riqi`. If born on 30th `tian`, then the `shengri` occurs on 29th in the `nian`s whose corresponding `yue` has only 29 `tian`s. If born in a `runyue`, the `shengri` occurs in the common non-`runyue` in the `nian`s that lack a corresponding `runyue`, or in the `runyue` in the `nian`s that do have one.

> One born in 1987-p01-01 celebrates the `shengri` on p01-01 every `nian`.\
> One born in 2004-p02-30 celebrates the `shengri` of 2024 `nian` on p02-29.\
> One born in 2004-r02-16 celebrates the `shengri` of 2022 `nian` on p02-16, and of 2023 `nian` on r02-16.

### `Jieqi`: a.k.a. Solar Terms

In this repository, `sui` starts from one `dongzhi` (a.k.a. winter solstice) until next. The ordinal of `jieqi` being 0, 1, ... 23 refers to `dongzhi`, `xiaohan`, ... `daxue`, where those with odd ordinals are classified as `jieling` and those with even ordinals as `zhongqi`.

> 1970 `sui` corresponds to the interval from Unix timestamp -861379 sec until 30695740 sec.

### Zodiac Sign

Zodiac signs do **NOT** belong to `nongli`, but are determined from the apparent longitude of the Sun on the ecliptic. Thus, they correspond to `jieqi`s in `nongli`. The ordinal of zodiac sign being 0, 1, ... 11 refers to Aries, Taurus, ... Pisces, respectively. Starting with Aries (`chunfen`, a.k.a. vernal equinox), each zodiac sign follows a `zhongqi` (`chunfen`, `guyu`, ... `yushui`).

### `Ganzhi`

`Ganzhi` is a combination of `tiangan` (`jia`, `yi`, ... `gui`) and `dizhi` (`zi`, `chou`, ... `hai`), occurring in pairs for 60-periodic counting. The ordinal of `ganzhi` being 0, 1, ... 59 refers to `jia_zi`, `yi_chou`, ... `gui_hai`, respectively.

### `Futian`

Counting from `xiazhi` day, `toufu` (or `chufu`) starts from the 3rd `geng` day, while `erfu` (or `zhongfu`) starts from the 4th `geng` day. Counting from `liqiu` day, `sanfu` (or `mofu`) starts from the 1st `geng` day. The durations of `toufu`, `erfu` and `sanfu` are 10 days, 10 or 20 days, and 10 days, respectively, totaling either 30 or 40 days.

### `Shujiu`

Counting from `dongzhi` day, each `jiu` consists of 9 consecutive days. A `sui` starts with such 9 `jiu`s, totaling 81 days.

> In Chinese, `jiu` means 9.

### `Bazi`

`Shengchen bazi` is determined from one's birth date and time, and consists of 4 `zhu`s, with 2 `zi`s (for `gan` and `zhi`) in each `zhu`. Traditionally, time was measured using sundials that display real solar time (RST). The time shown on the clock (mean solar time, MST) is converted to local time based on the birthplace's longitude, and then the Equation of Time (EoT) is added to obtain RST.

> In Chinese, `ba` means 8.

#### `Nian Zhu`

Each `nian zhu` follows a `lichun`. It is important to note that `shengxiao` (12 animals) corresponds with 12 `dizhi`s, but according to Chinese tradition, a `shengxiao` is recognized from a `chunjie` (p01-01).

> `Lichun` in 1984 `sui` is followed by a `jia_zi nian`.

#### `Yue Zhu`

Each `yue zhu` follows a `jieling` (`lichun`, `jingzhe`, ... `xiaohan`).

> `Daxue` in 2023 `sui` is followed by a `jia_zi yue`.

#### `Ri Zhu`

Each `ri zhu` follows a midnight (`zi_zheng`) in local RST.

> On the 120° E longitude, Unix timestamp 1735185600 sec falls within a `jia_zi ri`.

#### `Shi Zhu`

Each `shi zhu` follows an odd hour (`zi_chu` 23, `chou_chu` 1, ... `hai_chu` 21) in local RST.

> On the 120° E longitude, Unix timestamp 1738598400 sec falls within a `jia_zi shi`.

## Explanations

* Raw data are fetched from [SXWNL](https://www.sxwnl.com/super/).
* Daylight Saving Time (DST) in China from 1985 to 1991 is not taken into account, and Chinese Standard Time is treated as equivalent to UTC+8.
* The `bazi` calculations provided here are for reference only. Small errors around boundary instants may lead to biased or incorrect calculations.
* There is no scientific evidence to support the idea that `bazi` or zodiac signs can predict one's personality or destiny. Please approach these concepts with a critical mindset; this repository does not provide any warranties regarding the accuracy or applicability of its content.

## Acknowledgements

* Thanks to Mr. Xu Jianwei for having developed the website [SXWNL](https://www.sxwnl.com/super/).
* Thanks to Ms. Zhao Yuhan for providing insights of when to celebrate `shengri` on `nongli runyue`s.
