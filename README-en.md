# `Nongli`

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](README-zh.md)
[![en](https://img.shields.io/badge/lang-en-blue.svg)](README-en.md)

`Nongli` is a Chinese traditional calendar, depending on predicted positions of the Sun and the Moon.

> In Chinese, `nong` means agriculture and `li` means calendar.

[Purple Mountain Observatory, CAS](http://english.pmo.cas.cn/) is responsible for calculations and promulgations of `nongli` according to Chinese standard [GB/T 33661-2017](https://std.samr.gov.cn/gb/search/gbDetailed?id=71F772D817FDD3A7E05397BE0A0AB82A). As a lunisolar calendar, `nongli` not only reflects seasonality and phenology, but also indicates the moon phase and the tide, and thus is widely used in daily life and production, like agriculture, fishery, flood prevention, sailing, etc.

## Data

### Environment

```bash
conda env create -f data/environment.yml
conda activate Nongli
```

### Fetch Raw Data

Visit [SXWNL](https://www.sxwnl.com/super/) via Selenium, then calculate the instants of `shuo`s and `jieqi`s, and finally export the data to `data/build/raw.txt`. 

```bash
python3 -u data/spider.py
```

The constants `MIN` and `MAX` in the script are modifiable. Due to limited precisions of the fitting algorithms in SXWNL, the calculated data before -4172 A.D. (4713 B.C.) and those after 9999 A.D. are likely to be inaccurate.

### Rearrangements

The ordinals of `yue`s have changed some times in history. But for convenience, previous data are rearranged with the present calendar, and then exported into `data/build/`.

```bash
python3 -u data/split.py
```

The constants `MIN` and `MAX` in the notebook code are modifiable without exceeding the original data range. Note that this repository is NOT applicable for **historical** calendars **actually** used in and before 240s A.D.

### Generate C++ Data File

Open `data/coefs.ipynb` with Jupyter Notebook and export fitting arguments and residuals to `fit/data.hpp` by running all cells.

## Fit

All the fittings, except for EoT bias on calculation of `bazi`, perform integral additions, subtractions, multiplications and bitwise operations, attempting to avoid floating-point arithmetics and improve performances. Since C++20, right-shifts on signed integers are supposed to be arithmetic. Actually, most of C++ compilers perform right-shifts this way, and so is the user's compiler assumed even if it is compiling in C++17.

### Run test examples

```bash
mkdir fit/build/
cd fit/build/
cmake .. && make
./test.out
```

### `Riqi`: Date in `Nongli`

In this repository, `nian` comes from one `chunjie` (p01-01) until next. The value of `ryue` being 2, 3, ... 24, 25 refers to p01, r01, ... p12, r12 respectively. In Chinese, p11 and p12 are also called `dongyue` and `layue` respectively.

> 1970 `nian` corresponds to the interval from Unix timestamp 3081600 sec until 33753600 sec.

Here a `runyue` starts with an `r`, while a common non-`runyue` starts with a `p`.

### `Shengri`: Birthday in `Nongli`

In this repository, `shengri` comes when both `yue` and `tian` are identical to the birth `riqi`'s. If one was born on 30th `tian`, then the `shengri` comes on 29th in the `nian`s whose corresponding `yue` has 29 `tian`s only. If one was born in a `runyue`, then the `shengri` comes in the common non-`runyue` in the `nian`s who has no corresponding `runyue`, or in the `runyue` in the `nian`s who has one.

> One born in 1987-p01-01 celebrates the `shengri` on p01-01 every `nian`.

> One born in 2004-p02-30 celebrates the `shengri` of 2024 `nian` on p02-29.

> One born in 2004-r02-16 celebrates the `shengri` of 2022 `nian` on p02-16, and of 2023 `nian` on r02-16.

### `Jieqi`: a.k.a. Solar Term

In this repository, `sui` comes from one `dongzhi` (a.k.a. winter solstice) until next. The ordinal of `jieqi` being 0, 1, ... 23 refers to `dongzhi`, `xiaohan`, ... `daxue`, where those with odd ordinals are `jieling` and those with even ordinals are `zhongqi`.

> 1970 `sui` corresponds to the interval from Unix timestamp -861379 sec until 30695740 sec.

### Zodiac Sign

Zodiac signs do **NOT** belong to `nongli`, but are determined from the apparent longitude of the Sun on the ecliptic, and thus correspond to `jieqi`s in `nongli`. The ordinal of zodiac sign being 0, 1, ... 11 refers to Aries, Taurus, ... Pisces respectively. Starting with Aries (`chunfen`, a.k.a. vernal equinox), each zodiac sign comes following a `zhongqi` (`chunfen`, `guyu`, ... `yushui`).

### `Ganzhi`

`Ganzhi` is a combination of `tiangan` (`jia`, `yi`, ... `gui`) and `dizhi` (`zi`, `chou`, ... `hai`), and occurs in pairs for 60-periodic counting. The ordinal of `ganzhi` being 0, 1, ... 59 refers to `jia_zi`, `yi_chou`, ... `gui_hai` respectively.

### `Futian`

Counting from `xiazhi` day, `toufu` (or `chufu`) comes following the 3rd `geng` day; `erfu` (or `zhongfu`) comes following the 4th `geng` day. Counting from `liqiu` day, `sanfu` (or `mofu`) comes following the 1st `geng` day. `Toufu`, `erfu` and `sanfu` last 10 days, 10 or 20 days and 10 days respectively, 30 or 40 days in sum.

### `Shujiu`

Counting from `dongzhi` day, each `jiu` consists of 9 consecutive days. Each `sui` starts with such 9 `jiu`s (81 days).

> In Chinese, `jiu` means 9.

### `Bazi`

`Shengchen bazi` is determined from one's birth date and time, and consists of 4 `zhu`s, with 2 `zi`s (for `gan` and `zhi`) in each `zhu`. People used to measure time thru sundials displaying real solar time (RST). The time shown on the clock (i.e. mean solar time, MST) is supposed to convert to the local time according to the birthplace longitude, and then add the equation of time (EoT) estimated via astronomical algorithms. This way RST comes out by adding EoT to local MST.

#### `Nian Zhu`

Each `nian zhu` comes following a `lichun`. It is worth mentioning that `shengxiao` (12 animals) matches with 12 `dizhi`s, but according to Chinese folks, a `shengxiao` comes following a `chunjie` (p01-01).

> `Lichun` in 1984 `sui` is followed by a `jia_zi nian`.

#### `Yue Zhu`

Each `yue zhu` comes following a `jieling` (`lichun`, `jingzhe`, ... `xiaohan`).

> `Daxue` in 2023 `sui` is followed by a `jia_zi yue`.

#### `Ri Zhu`

Each `ri zhu` comes following a midnight (`zi_zheng`) in local RST.

> On 120° E, Unix timestamp 1735185600 sec is contained in a `jia_zi ri`.

#### `Shi Zhu`

Each `shi zhu` comes following an odd hour (`zi_chu` 23, `chou_chu` 1, ... `hai_chu` 21) in local RST.

> On 120° E, Unix timestamp 1738598400 sec is contained in a `jia_zi shi`.

## Explanations

* Raw data are fetched from [SXWNL](https://www.sxwnl.com/super/).
* Daylight saving time (DST) in China from 1985 to 1991 are taken out of account, and Chinese Standard Time is seen equivalent to UTC+8.
* The `bazi` is estimated here for reference only. Around the boundary instants, a small error might lead to a biased or wrong calculation.
* There has not been any evidence if `bazi` and zodiac sign are related to predicting one's personality or destiny. Please do not be superstitious, and this repository has no warranty of any kind.

## Acknowledgements

* Thanks to Mr. Xu Jianwei for having developed the website [SXWNL](https://www.sxwnl.com/super/).
* Thanks to Ms. Zhao Yuhan for providing details of when to celebrate birthdays on `nongli runyue`s.
