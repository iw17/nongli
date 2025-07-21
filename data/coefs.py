import argparse as ap
import ctypes as ct
import os
import time
import typing as tp
import warnings

import numpy as np
import pandas as pd
from numpy.polynomial import polynomial as poly


# Part 0: Utilities


UInt8s: tp.TypeAlias = np.typing.NDArray[np.uint8]
Int32s: tp.TypeAlias = np.typing.NDArray[np.int32]
Int64s: tp.TypeAlias = np.typing.NDArray[np.int64]
Floats: tp.TypeAlias = np.typing.NDArray[np.float64]


def int_frac(x: float) -> tuple[int, float]:
    return int(x // 1), float(x % 1)


def poly_fit(x: tp.Any, y: tp.Any, deg: int) -> Floats:
    x, y = map(np.asarray, [x, y])
    coefs: tp.Any = poly.polyfit(x, y, deg)
    return tp.cast(Floats, coefs)[::-1]


class Config(tp.NamedTuple):
    '''
    Configurations for time bounds to export.

    Assumptions:
        * `upper > lower`.

    Attributes:
        lower (int): minimal `nian`, `sui` and year
        upper (int): maximal `nian`, `sui` and year
        output (str): path to output HPP data file
    '''

    lower: int
    upper: int
    output: str


def load_config() -> Config:
    parser: ap.ArgumentParser = ap.ArgumentParser()
    kw_min: dict[str, tp.Any] = {
        'type': int,
        'required': True,
        'help': 'lower bound of nian, sui, year',
        'metavar': 'MIN',
    }
    kw_max: dict[str, tp.Any] = {
        'type': int,
        'required': True,
        'help': 'upper bound of nian, sui, year',
        'metavar': 'MAX',
    }
    kw_out: dict[str, tp.Any] = {
        'type': str,
        'default': '',
        'help': 'path to output HPP data file',
        'metavar': 'PATH',
    }
    parser.add_argument('-l', '--lower', **kw_min)
    parser.add_argument('-u', '--upper', **kw_max)
    parser.add_argument('-o', '--output', **kw_out)
    args: ap.Namespace = parser.parse_args()
    return Config(args.lower, args.upper, args.output)


class Format:
    '''
    Constants of formatting exported data.

    Attributes:
        BPL (int): bytes per line
        LPA (int): lines per array for huge tables
        NPL (int): names per line for array pointers
        XX_BIT (int): bits per item of (NR, NY, YD, JS)
        XX_IPB (int): items per byte of (NR, NY, YD)
    '''

    BPL: int = 12
    LPA: int = 648
    NPL: int = 5
    NR_BIT, NR_IPB = 4, 2 # `nian` to `run`
    NY_BIT, NY_IPB = 1, 8 # `nian` to `cyue`
    YD_BIT, YD_IPB = 2, 4 # `cyue` to uday
    JS_BIT: int = 12 # `cjie` to usec


def vals_to_colls(vals: list[int], bit: int, ipc: int) -> list[int]:
    '''
    Converts a list of integers to collections (e.g. bytes).

    Assumptions:
        * All items in `vals` are non-negative.
        * No overflows, especially `bit * ipc < 64`.

    Args:
        vals (list[int]): integer values to convert
        bit (int): how many bits a value item occupies
        ipc (int): how many items a collection contains
    Returns:
        list[int]: list of small integers for collections
    '''

    UInt64s = np.typing.NDArray[np.uint64]
    arr: UInt64s = np.array(vals, dtype=np.uint64)
    arr = arr.reshape((-1, ipc))
    exp: UInt64s = bit * np.arange(ipc, dtype=np.uint64)
    sums: UInt64s = np.sum(arr << exp, axis=1)
    return tp.cast(list[int], sums.tolist())


class Data(tp.NamedTuple):
    '''
    Lunar and solar data preprocessed into CSV.

    Attributes:
        lunar (pd.DataFrame): info on `cyue`s and `shuo`s
        solar (pd.DataFrame): info on `sui`s and `jieqi`s
    '''

    lunar: pd.DataFrame
    solar: pd.DataFrame


def load_data(csv_dir: str) -> Data:
    '''
    Loads lunar and solar data from CSVs.
    '''

    lu_path: str = os.path.join(csv_dir, 'lunar.csv')
    lu: pd.DataFrame = pd.read_csv(lu_path)
    so_path: str = os.path.join(csv_dir, 'solar.csv')
    so: pd.DataFrame = pd.read_csv(so_path)
    return Data(lu, so)


def trim_data(data: Data, conf: Config) -> Data:
    '''
    Removes lunar and solar data out of bounds.
    '''

    (lu, so), (lo, hi, _) = data, conf
    # lunar: `nian` and `cyue`
    lu_fi: pd.Series = lu['nian'].between(lo, hi)
    # aligns `nian` to whole line
    nian_no: int = hi - lo + 1
    NIAN_MOD: int = Format.NY_IPB * Format.BPL
    nian_no += NIAN_MOD - nian_no % NIAN_MOD
    nian_hi: int = lo + nian_no - 1
    lu_fi[lu['nian'].between(lo, nian_hi)] = True
    # aligns `cyue` to whole line
    cyue_lo: int = int(lu['cyue'][lu_fi].min())
    cyue_no: int = int(lu_fi.sum())
    CYUE_MOD: int = Format.YD_IPB * Format.BPL
    cyue_no += CYUE_MOD - cyue_no % CYUE_MOD
    cyue_hi: int = cyue_lo + cyue_no - 1
    lu_fi[lu['cyue'].between(cyue_lo, cyue_hi)] = True
    # aligns the last `nian`
    nian_hi = int(lu['nian'][lu_fi].max())
    lu_fi[lu['nian'] == nian_hi] = True
    lu = lu[lu_fi]
    # adds a new column uday
    with warnings.catch_warnings(action='ignore'):
        lu['uday'] = (lu['usec'] + 28800) // 86400
    # solar: `sui` and `jieqi`
    so = so[so['sui'].between(lo, hi + 1)]
    return Data(lu, so)


# Part 1: Lunar Data on `riqi`


## Section 1.1. (`nian`, `ryue`) to `cyue` to uday


### 1.1.0. Preparations


class CoefsLv1(tp.NamedTuple):
    '''
    Fit form: `y = k0 * x + b0 + (k1 * x + b1 >> nb)`.

    Attributes:
        k0 (int): major linear
        b0 (int): major constant
        k1 (int): minor linear
        b1 (int): minor constant
        nb (int): bits shifted
    '''

    k0: int
    b0: int
    k1: int
    b1: int
    nb: int

    def predict(self: tp.Self, xs: Int64s) -> Int64s:
        k0, b0, k1, b1, nb = self
        return k0 * xs + b0 + (k1 * xs + b1 >> nb)


class Lv1CR(tp.NamedTuple):
    coefs: CoefsLv1
    resis: list[int]


def lv1_fit(xs: pd.Series, ys: pd.Series, bit: int) -> Lv1CR:
    '''
    Fits `ys` from `xs` like `CoefsLv1`.

    Args:
        xs (pd.Series): data input to fit
        ys (pd.Series): expected output of data
        bit (int): max bit length of one residual
    Returns:
        Lv1CR: coefficients and residuals
    Raises:
        ValueError: if no feasible fit was found
    '''

    # fits like `y = k * x + b`
    kb: Floats = poly_fit(xs, ys, deg=1)
    (k0, kf), (b0, bf) = map(int_frac, kb.tolist())
    # finds least feasible bits
    for nb in range(32):
        k1: int = round(kf * (1 << nb))
        b1: int = round(bf * (1 << nb))
        coefs: CoefsLv1 = CoefsLv1(k0, b0, k1, b1, nb)
        xa: Int64s = xs.to_numpy().astype(np.int64)
        rs: pd.Series = ys - coefs.predict(xa)
        rmin: int = int(rs.min())
        coefs = CoefsLv1(k0, b0 + rmin, k1, b1, nb)
        if rs.max() - rmin < (1 << bit):
            return Lv1CR(coefs, (rs - rmin).tolist())
    else:
        raise ValueError('bad idea, residuals too wide')


### 1.1.1. NR: `nian` to `run`: direct export


def nr_runs(lu: pd.DataFrame) -> list[int]:
    '''
    Gets `runyue` values of `nian`s in bounds.

    Returns:
        list[int]: `run`s of `nian`s, 13 for `run`-free
    '''

    lo, hi = lu['nian'].min(), lu['nian'].max()
    num: int = int(hi) - int(lo) + 1
    NIAN_MOD: int = Format.NR_IPB * Format.BPL
    hi = lo + (num // NIAN_MOD) * NIAN_MOD - 1
    lu = lu[(lu['ryue'] & 1 != 0) & lu['nian'].between(lo, hi)]
    nians, ryues = lu['nian'], lu['ryue'] // 2
    rmap: dict[int, int] = dict(zip(nians, ryues))
    return [rmap.get(n, 13) for n in range(lo, hi + 1)]


def nr_bytes(runs: list[int]) -> list[int]:
    '''
    Bitwise converts `runyue` values to bytes.

    Args:
        runs (list[int]): `run`s of `nian`s, or 13
    Returns:
        list[int]: bytes converted from `runyue`s
    '''

    return vals_to_colls(runs, Format.NR_BIT, Format.NR_IPB)


### 1.1.2. NY: `nian` to `cyue`: linear (1-deg) regression


def ny_fit(lu: pd.DataFrame) -> Lv1CR:
    '''
    Fits `nian_to_cyue` and gets residuals in `cyue`.
    '''

    lo, hi = lu['nian'].min(), lu['nian'].max()
    num: int = int(hi) - int(lo) + 1
    NIAN_MOD: int = Format.NY_IPB * Format.BPL
    hi = lo + (num // NIAN_MOD) * NIAN_MOD - 1
    lu = lu[(lu['ryue'] == 2) & lu['nian'].between(lo, hi)]
    return lv1_fit(lu['nian'], lu['cyue'], Format.NY_BIT)


def ny_bytes(resy: list[int]) -> list[int]:
    '''
    Bitwise converts `nian_to_cyue` residuals to bytes.

    Args:
        resy (list[int]): NY residuals in `cyue`
    Returns:
        list[int]: bytes converted from NY residuals
    '''

    return vals_to_colls(resy, Format.NY_BIT, Format.NY_IPB)


### 1.1.3. YD: `cyue` to uday: linear (1-deg) regression


def yd_fit(lu: pd.DataFrame) -> Lv1CR:
    '''
    Fits `cyue_to_uday` and gets residuals in uday.
    '''

    lo, hi = lu['cyue'].min(), lu['cyue'].max()
    num: int = int(hi) - int(lo) + 1
    CYUE_NUM: int = Format.YD_IPB * Format.BPL
    hi = lo + (num // CYUE_NUM) * CYUE_NUM - 1
    lu = lu[lu['cyue'].between(lo, hi)]
    return lv1_fit(lu['cyue'], lu['uday'], Format.YD_BIT)


def yd_bytes(resd: list[int]) -> list[int]:
    '''
    Bitwise converts `cyue_to_uday` residuals to bytes.

    Args:
        resd (list[int]): YD residuals in uday
    Returns:
        list[int]: bytes converted from YD residuals
    '''

    return vals_to_colls(resd, Format.YD_BIT, Format.YD_IPB)


## Section 1.2. uday to `cyue` to `nian`, no residuals


### 1.2.0. Preparations


class CoefsExact(tp.NamedTuple):
    '''
    Fit form: `y = b0 + (k1 * x + b1 >> nb)`.

    Attributes:
        b0 (int): major constant
        k1 (int): minor linear
        b1 (int): minor constant
        nb (int): bits shifted
    '''

    b0: int
    k1: int
    b1: int
    nb: int

    def predict(self: tp.Self, xs: Int64s) -> Int64s:
        b0, k1, b1, nb = self
        return b0 + (k1 * xs + b1 >> nb)


def exact_fit(xs: pd.Series, ys: pd.Series) -> CoefsExact:
    # fits like `y = k * x + b`
    k, b = poly_fit(xs, ys + 0.5, deg=1)
    b0, bf = int_frac(b)
    # finds least feasible bits
    for nb in range(32):
        k1: int = round(float(k) * (1 << nb))
        b1: int = round(bf * (1 << nb))
        coefs: CoefsExact = CoefsExact(b0, k1, b1, nb)
        xa: Int64s = xs.to_numpy().astype(np.int64)
        rs: Int64s = ys.to_numpy() - coefs.predict(xa)
        rmax: int = int(rs.max())
        if rmax - rs.min() == 0:
            return CoefsExact(b0 + rmax, k1, b1, nb)
    else:
        raise ValueError('bad idea, residuals too wide')


### 1.2.1. DY: uday to `cyue`: linear (1-deg) regression


def dy_fit(lu: pd.DataFrame) -> CoefsExact:
    return exact_fit(lu['uday'], lu['cyue'])


### 1.2.2. YN: `cyue` to `nian`: linear (1-deg) regression


def yn_fit(lu: pd.DataFrame) -> CoefsExact:
    p01_fi: pd.Series = lu['ryue'] == 2
    p01_cyue: pd.Series = lu['cyue'][p01_fi]
    p01_nian: pd.Series = lu['nian'][p01_fi]
    return exact_fit(p01_cyue, p01_nian)


# Part 2: Solar Data on `jieqi`


## Section 2.1. `shihou` to usec


### 2.1.1. JS: (`sui`, `jie`) to usec: sextic (6-deg) regression


class CoefsSolar(tp.NamedTuple):
    '''
    Fit form: `y = k0 * x + b0 + s(x)` with sextic minor like
    `s(x) = (...((c[0] * x >> nb) + c[1] ) * ... >> nb) + c[6]`.

    Attributes:
        k0 (int): major linear
        b0 (int): major constant
        nb (int): bits shifted per level
        cs (Int64s): (24, 7)-shaped minors
    '''

    k0: int
    b0: int
    nb: int
    cs: Int64s

    def predict(self: tp.Self, xs: Int64s) -> Int64s:
        k0, b0, nb, cs = self
        suis: Int64s = xs // 24 + 1970
        jies: Int64s = xs % 24
        plin: Int64s = k0 * suis + b0
        coef: Int64s = cs[jies].T
        pfit: Int64s = coef[0]
        pfit = (pfit * suis >> nb) + coef[1]
        pfit = (pfit * suis >> nb) + coef[2]
        pfit = (pfit * suis >> nb) + coef[3]
        pfit = (pfit * suis >> nb) + coef[4]
        pfit = (pfit * suis >> nb) + coef[5]
        pfit = (pfit * suis >> nb) + coef[6]
        return plin + pfit


class SolarCR(tp.NamedTuple):
    coefs: CoefsSolar
    resis: list[int]


def js_fit(so: pd.DataFrame) -> SolarCR:
    '''
    Fits `cjie_to_usec` and gets residuals in usec.
    '''

    suis: pd.Series = so['sui']
    usec: pd.Series = so['usec']
    # linear major
    kb: Floats = poly_fit(suis, usec, deg=1)
    (k0, _), (b0, _) = map(int_frac, kb.tolist())
    rsec: pd.Series = usec - (k0 * suis + b0)
    # sextic minor
    cjies: Int64s = so['cjie'].to_numpy()
    for nb in range(32):
        lscf: list[Int64s] = [] # list of coefficients
        for jie in range(24):
            # items of the specific `jieqi`
            ssui: Floats = suis[so['jie'] == jie].to_numpy()
            ssec: Floats = rsec[so['jie'] == jie].to_numpy()
            coef: Floats = poly_fit(ssui, ssec, deg=6)
            coef *= (2.0 ** (nb * np.arange(7)))[::-1]
            lscf.append(coef.astype(np.int64))
        cs: Int64s = np.array(lscf).astype(np.int64)
        coefs: CoefsSolar = CoefsSolar(k0, b0, nb, cs)
        ress: pd.Series = usec - coefs.predict(cjies)
        rmin: int = int(ress.min())
        if ress.max() - rmin < (1 << Format.JS_BIT):
            c0: Int64s = cs[:, -1]
            bm: int = int(c0.max() + c0.min()) >> 1
            b0, cs[:, -1] = b0 + bm, c0 - bm
            coefs = CoefsSolar(k0, b0 + rmin, nb, cs)
            return SolarCR(coefs, (ress - rmin).tolist())
    else:
        raise ValueError('bad idea, residuals too wide')


def js_bytes(ress: list[int]) -> list[int]:
    '''
    Bitwise converts `shihou_to_usec` residuals to bytes.

    Args:
        ress (list[int]): JS residuals in usec
    Returns:
        list[int]: bytes converted from JS residuals
    '''

    colls: list[int] = []
    for trib in vals_to_colls(ress, bit=12, ipc=2):
        lo: int = trib & 0xff
        md: int = (trib >> 8) & 0xff
        hi: int = trib >> 16
        colls.extend([lo, md, hi])
    return colls


## Section 2.2. usec to `shihou`: linear (1-deg) regression


### 2.2.0. Preparations


class CoefsLv2(tp.NamedTuple):
    '''
    Fit form: y = b0 + ((k1 * x >> nb) + b1 >> nb).

    Attributes:
        b0 (int): major constant
        k1 (int): minor quadratic
        b1 (int): minor linear
        nb (int): bits shifted
    '''

    b0: int
    k1: int
    b1: int
    nb: int

    def predict(self: tp.Self, xs: Int64s) -> Int64s:
        b0, k1, b1, nb = self
        return b0 + ((k1 * xs >> nb) + b1 >> nb)


def lv2_fit(xs: pd.Series, ys: pd.Series) -> CoefsLv2:
    xa: Int64s = xs.to_numpy(dtype=np.int64)
    ya: Int64s = ys.to_numpy(dtype=np.int64)
    for nb in range(22): # avoids overflow for huge x
        kb: Floats = poly_fit(xs, ys + 0.5, deg=1)
        kb *= 2.0 ** (nb * (1 + np.arange(2))[::-1])
        k1: int = round(float(kb[0]))
        b1: int = round(float(kb[1]))
        b0: int = round(b1 / (1 << nb))
        b1 -= b0 << nb
        coefs: CoefsLv2 = CoefsLv2(b0, k1, b1, nb)
        rs: Int64s = ya - coefs.predict(xa)
        rmax: int = int(rs.max())
        if rmax - rs.min() == 0:
            return CoefsLv2(b0 + rmax, k1, b1, nb)
    else:
        raise ValueError('bad idea, residuals too wide')


### 2.2.1. SJ: usec to `cjie`: linear (1-deg) regression


def sj_fit(so: pd.DataFrame) -> CoefsLv2:
    return lv2_fit(so['usec'], so['cjie'])


# Part 3: Export to HPP data file


## Section 3.0. Preparations


ICX: tp.LiteralString = 'inline constexpr'


def write_head(hpp: str, conf: Config, data: Data) -> int:
    '''
    Writes head and time bounds into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        conf (Config): time bounds configuration
        data (Data): lunar and solar data loaded
    Returns:
        int: total bytes of variables into C++
    '''

    (lo, hi, _), (lu, so) = conf, data
    lf: pd.Series = lu['nian'].between(lo, hi) # lunar filter
    ym: int = int(lu[lf]['cyue'].min()) # cyue min
    yM: int = int(lu[lf]['cyue'].max()) # cyue max
    sf: pd.Series = so['sui'].between(lo, hi) # solar filter
    jm: int = int(so[sf]['cjie'].min()) # cjie min
    jM: int = int(so[sf]['cjie'].max()) # cjie max
    with open(hpp, 'a') as hpp_out:
        hpp_out.write('#ifndef IW_DATA_HPP\n')
        hpp_out.write('#define IW_DATA_HPP 20251005L\n\n')
        hpp_out.write('#include <cstdint>\n\n')
        hpp_out.write('namespace iw17::data {\n\n')
        hpp_out.write('inline namespace limits {\n\n')
        hpp_out.write(f'{ICX} int16_t NIAN_MIN = {lo:d};\n')
        hpp_out.write(f'{ICX} int16_t NIAN_MAX = {hi:d};\n\n')
        hpp_out.write(f'{ICX} int16_t SUI_MIN = {lo:d};\n')
        hpp_out.write(f'{ICX} int16_t SUI_MAX = {hi:d};\n\n')
        hpp_out.write(f'{ICX} int16_t YEAR_MIN = {lo:d};\n')
        hpp_out.write(f'{ICX} int16_t YEAR_MAX = {hi:d};\n\n')
        hpp_out.write(f'{ICX} int32_t CYUE_MIN = {ym:d};\n')
        hpp_out.write(f'{ICX} int32_t CYUE_MAX = {yM:d};\n\n')
        hpp_out.write(f'{ICX} int32_t CJIE_MIN = {jm:d};\n')
        hpp_out.write(f'{ICX} int32_t CJIE_MAX = {jM:d};\n\n')
        hpp_out.write('} // namespace limits\n\n')
    return 2 * 6 + 4 * 4


def fmt_bytes(arr: list[int], bpl: int, tab: int = 0) -> str:
    '''
    Produces string ready for export from byte array.

    Args:
        arr (list[int]): byte array to export
        bpl (int): bytes per line in the string
        tab (int): indents leading each line, default 0
    Returns:
        str: string formatted for export to C++
    
    Examples:
        >>> fmt_bytes([18, 217, 34, 12, 75, 60], bpl=3)
        0x12, 0xd9, 0x22,
        0x0c, 0x4b, 0x3c,
    '''

    idt: str = ' ' * (4 * tab) # indent
    fmt: str = ' '.join(['0x{:02x},'] * bpl) + '\n'
    lns: list[str] = [] # lines
    for i in range(0, len(arr), bpl):
        lns.append(idt + fmt.format(*arr[i:i+bpl]))
    return ''.join(lns)


def fmt_coef_arr(arr: tp.Sequence[int]) -> str:
    '''
    Produces string ready for export from coefs array.

    Args:
        arr (Int64s): coefficient array to export
    Returns:
        str: string formatted for export to C++

    Examples:
        >>> fmt_coef_arr([1, 2, 3])
        { 1, 2, 3 }
    '''

    
    fmt: str = ', '.join(['{:d}'] * len(arr))
    return '{ ' + fmt.format(*arr) + ' }'


def fmt_coef_mat(mat: Int64s, tab: int = 0) -> str:
    '''
    Produces string ready for export from coefs matrix.

    Args:
        mat (Int64s): coefficient matrix to export
        tab (int): indents leading each line, default 0
    Returns:
        str: string formatted for export to C++

    >>> fmt_coef_mat(np.array([[7, 24], [-5, 8]]))
    {  7, 24 },
    { -5,  8 },
    '''

    row, col = mat.shape
    idt: str = ' ' * (4 * tab) # indent
    sls: Int64s = np.char.str_len(mat.astype(str))
    fmt: str = ', '.join([R'{{:{:d}d}}'] * col)
    fmt = '{{ ' + fmt.format(*sls.max(axis=0)) + ' }},'
    lns: list[str] = [] # lines
    for ln in tp.cast(list[list[int]], mat.tolist()):
        lns.append(idt + fmt.format(*ln) + '\n')
    return ''.join(lns)


def write_tail(hpp: str, total: int) -> None:
    '''
    Writes tail along with total bytes in C++.

    Args:
        hpp (str): path to HPP file exporting to
        total (int): total bytes of variables exported
    '''

    with open(hpp, 'a') as hpp_out:
        hpp_out.write('} // namespace iw17::data, ')
        hpp_out.write(f'{total} bytes in total\n\n')
        hpp_out.write('#endif // IW_DATA_HPP\n')


## Section 3.1. Lunar: NR, NY, YD, DY, YN


### 3.1.1. NR: `nian_to_run` data


def write_nr(hpp: str, runs: list[int]) -> int:
    '''
    Writes `nian_to_run` data into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        runs (list[int]): bytes of `runyue` data
    Returns:
        int: total bytes of variables into C++
    '''

    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} uint8_t NR_RUNS[] = {{\n')
        hpp_out.write(fmt_bytes(runs, Format.BPL, tab=1))
        hpp_out.write('}; // NR_RUNS\n\n')
    return len(runs)


### 3.1.2. NY: `nian_to_cyue` coefs and resy


def write_ny_coef(hpp: str, coefs: CoefsLv1) -> int:
    '''
    Writes fitting coefs of NY into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        coefs (CoefsLv1): coefficients of NY fitting
    Returns:
        int: total bytes of variables into C++
    '''

    *cs, nb = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t NY_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t NY_COEF[] = ')
        hpp_out.write(fmt_coef_arr(cs))
        hpp_out.write(';\n\n')
    return 8 * len(coefs)


def write_ny_resy(hpp: str, resy: list[int]) -> int:
    '''
    Writes residuals of NY into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        resy (list[int]): bytes of NY residuals
    Returns:
        int: total bytes of variables into C++
    '''

    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} uint8_t NY_RESY[] = {{\n')
        hpp_out.write(fmt_bytes(resy, Format.BPL, tab=1))
        hpp_out.write('}; // NY_RESY\n\n')
    return len(resy)


### 3.1.3. YD: `cyue_to_uday` coefs and resd


def write_yd_coef(hpp: str, coefs: CoefsLv1) -> int:
    '''
    Writes fitting coefs of YD into HPP file.

    Returns:
        int: total bytes of variables into C++
    '''

    *cs, nb = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t YD_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t YD_COEF[] = ')
        hpp_out.write(fmt_coef_arr(cs))
        hpp_out.write(';\n\n')
    return 8 * len(coefs)


def write_yd_resd(hpp: str, resd: list[int]) -> int:
    '''
    Writes page-wise residuals of YD into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        resd (list[int]): bytes of YD residuals
    Returns:
        int: total bytes of variables into C++
    '''

    BPA: int = Format.BPL * Format.LPA # bytes per array
    len_res: int = len(resd)
    pages: int = len_res // BPA + bool(len_res % BPA)
    max_len: int = len(str(pages - 1))
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t YD_PAGE = {BPA};\n\n')
    arr_fmt: str = f'YD_RESD_{{:0{max_len}d}}'
    for i, lo in enumerate(range(0, len_res, BPA)):
        arr: str = arr_fmt.format(i)
        slc: list[int] = resd[lo:lo+BPA]
        with open(hpp, 'a') as hpp_out:
            hpp_out.write(f'{ICX} uint8_t {arr}[] = {{\n')
            hpp_out.write(fmt_bytes(slc, Format.BPL, tab=1))
            hpp_out.write(f'}}; // {arr}\n\n')
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} const uint8_t *YD_ARRD[] = {{\n')
        for i in range(0, pages, Format.NPL):
            ln_num: int = min(Format.NPL, pages - i)
            ln_fmt: str = ', '.join([arr_fmt] * ln_num) + ','
            ln: str = ln_fmt.format(*range(i, i + ln_num))
            hpp_out.write(f'    {ln}\n')
        hpp_out.write('}; // YD_ARRD\n\n')
    return 8 + len(resd) + ct.sizeof(ct.c_void_p) * pages


### 3.1.4. DY: `uday_to_cyue` coefficients


def write_dy_coef(hpp: str, coefs: CoefsExact) -> int:
    '''
    Writes fitting coefs of DY into HPP file.

    Returns:
        int: total bytes of variables into C++
    '''

    *cs, nb = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t DY_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t DY_COEF[] = ')
        hpp_out.write(fmt_coef_arr(cs))
        hpp_out.write(';\n\n')
    return 8 * len(coefs)


### 3.1.5. YN: `cyue_to_nian` coefficients


def write_yn_coef(hpp: str, coefs: CoefsExact) -> int:
    '''
    Writes fitting coefs of YN into HPP file.

    Returns:
        int: total bytes of variables into C++
    '''

    *cs, nb = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t YN_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t YN_COEF[] = ')
        hpp_out.write(fmt_coef_arr(cs))
        hpp_out.write(';\n\n')
    return 8 * len(coefs)


## Section 3.2. Solar: JS, SJ


### 3.2.1. JS: `shihou_to_usec` coefs and ress


def write_js_coef(hpp: str, coefs: CoefsSolar) -> int:
    '''
    Writes fitting coefs of JS into HPP file.

    Returns:
        int: total bytes of variables into C++
    '''

    k0, b0, nb, cs = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t JS_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t JS_CLIN[] = ')
        hpp_out.write(fmt_coef_arr([k0, b0]))
        hpp_out.write(';\n\n')
        hpp_out.write(f'{ICX} int64_t JS_COEF[][7] = {{\n')
        hpp_out.write(fmt_coef_mat(cs, tab=1))
        hpp_out.write('}; // JS_COEF\n\n')
    return 8 * (len(coefs) - 1) + 8 * int(np.prod(cs.shape))


def write_js_ress(hpp: str, ress: list[int]) -> int:
    '''
    Writes page-wise residuals of JS into HPP file.

    Args:
        hpp (str): path to HPP file exporting to
        ress (list[int]): bytes of JS residuals
    Returns:
        int: total bytes of variables into C++
    '''

    BPA: int = Format.BPL * Format.LPA # bytes per array
    len_res: int = len(ress)
    pages: int = len_res // BPA + bool(len_res % BPA)
    max_len: int = len(str(pages - 1))
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t JS_PAGE = {BPA};\n\n')
    arr_fmt: str = f'JS_RESS_{{:0{max_len}d}}'
    for i, lo in enumerate(range(0, len_res, BPA)):
        arr: str = arr_fmt.format(i)
        slc: list[int] = ress[lo:lo+BPA]
        with open(hpp, 'a') as hpp_out:
            hpp_out.write(f'{ICX} uint8_t {arr}[] = {{\n')
            hpp_out.write(fmt_bytes(slc, Format.BPL, tab=1))
            hpp_out.write(f'}}; // {arr}\n\n')
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} const uint8_t *JS_ARRS[] = {{\n')
        for i in range(0, pages, Format.NPL):
            ln_num: int = min(Format.NPL, pages - i)
            ln_fmt: str = ', '.join([arr_fmt] * ln_num) + ','
            ln: str = ln_fmt.format(*range(i, i + ln_num))
            hpp_out.write(f'    {ln}\n')
        hpp_out.write('}; // JS_ARRS\n\n')
    return len(ress) + ct.sizeof(ct.c_void_p) * pages


### 3.2.2. SJ: `usec_to_shihou` coefficients


def write_sj_coef(hpp: str, coefs: CoefsLv2) -> int:
    '''
    Writes fitting coefs of SJ into HPP file.

    Returns:
        int: total bytes of variables into C++
    '''

    *cs, nb = coefs
    with open(hpp, 'a') as hpp_out:
        hpp_out.write(f'{ICX} int64_t SJ_BITS = {nb};\n\n')
        hpp_out.write(f'{ICX} int64_t SJ_COEF[] = ')
        hpp_out.write(fmt_coef_arr(cs))
        hpp_out.write(';\n\n')
    return 8 * len(coefs)


# Part 4: Export


def main() -> None:
    t0: float = time.perf_counter()
    conf: Config = load_config()
    here: str = os.path.dirname(__file__)
    csv_dir: str = os.path.join(here, 'build')
    data: Data = load_data(csv_dir)
    # makes `js_fit` well-conditioned
    js_so: pd.DataFrame = data.solar
    if conf.lower > 0:
        lo, hi = 0, conf.upper
    elif conf.upper < 0:
        lo, hi = conf.lower, 0
    else:
        lo, hi = conf[:2]
    js_so = js_so[js_so['sui'].between(lo, hi + 1)]
    js_cf, js_rs = js_fit(js_so)
    lo, hi, hpp = conf
    js_fi: pd.Series = js_so['sui'].between(lo, hi)
    js_ra: Int64s = np.array(js_rs)[js_fi]
    js_rs = tp.cast(list[int], js_ra.tolist())
    data = trim_data(data, conf)
    lunar, solar = data
    # head
    if not hpp:
        hpp = here
    if os.path.isdir(hpp):
        hpp = os.path.join(hpp, 'data.hpp')
    with open(hpp, 'w'):
        pass
    total: int = 0
    total += write_head(hpp, conf, data)
    # `nian_to_run`
    nr_rs: list[int] = nr_runs(lunar)
    nr_bt: list[int] = nr_bytes(nr_rs)
    total += write_nr(hpp, runs=nr_bt)
    # `nian_to_cyue`
    ny_cf, ny_ry = ny_fit(lunar)
    total += write_ny_coef(hpp, coefs=ny_cf)
    ny_bt: list[int] = ny_bytes(ny_ry)
    total += write_ny_resy(hpp, resy=ny_bt)
    # `cyue_to_uday`
    yd_cf, yd_rd = yd_fit(lunar)
    total += write_yd_coef(hpp, coefs=yd_cf)
    yd_bt: list[int] = yd_bytes(yd_rd)
    total += write_yd_resd(hpp, resd=yd_bt)
    # `uday_to_cyue`
    dy_cf: CoefsExact = dy_fit(lunar)
    total += write_dy_coef(hpp, coefs=dy_cf)
    # `cyue_to_nian`
    yn_cf: CoefsExact = yn_fit(lunar)
    total += write_yn_coef(hpp, coefs=yn_cf)
    # `shihou_to_usec`
    total += write_js_coef(hpp, coefs=js_cf)
    js_bt: list[int] = js_bytes(js_rs)
    total += write_js_ress(hpp, ress=js_bt)
    # `usec_to_shihou`
    sj_cf: CoefsLv2 = sj_fit(solar)
    total += write_sj_coef(hpp, coefs=sj_cf)
    # tail
    t1: float = time.perf_counter()
    secs: float = t1 - t0
    write_tail(hpp, total)
    hpp = os.path.abspath(hpp)
    print(f'data exported to \"{hpp}\", {secs:.3f} s taken')


if __name__ == '__main__':
    main()