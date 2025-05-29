import argparse as ap
import os
import typing as tp

import numpy as np
import pandas as pd


# Part 0: Utilities


UInt8s: tp.TypeAlias = np.typing.NDArray[np.uint8]
Int32s: tp.TypeAlias = np.typing.NDArray[np.int32]
Int64s: tp.TypeAlias = np.typing.NDArray[np.int64]
Floats: tp.TypeAlias = np.typing.NDArray[np.float64]


def int_frac(x: float) -> tuple[int, float]:
    '''
    Splits into integral and fractional parts.

    Args:
        x (float): real number to split
    Returns:
        int, float: int and frac parts of `x`

    Examples:
        >>> int_frac(1.5)
        (1, 0.5)
        >>> int_frac(-3.75)
        (-4, 0.25)
    '''

    return int(x // 1), x % 1


class Config(tp.NamedTuple):
    '''
    Configurations for time bounds to export.

    Assumptions:
        * `upper` is not less than `lower`.

    Attributes:
        lower (int): minimal `nian`, `sui` and year
        upper (int): maximal `nian`, `sui` and year
        tzinfo (float): offset hours eastward from UTC
    '''

    lower: int
    upper: int
    tzinfo: float


def load_config() -> Config:
    '''
    Loads configurations from command-line.

    Returns:
        Config: bounds and tzinfo input from command-line
    '''

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
    kw_tz: dict[str, tp.Any] = {
        'type': int,
        'default': 8.0,
        'help': 'offset hours eastward from UTC',
        'metavar': 'TZ',
    }
    parser.add_argument('-l', '--lower', **kw_min)
    parser.add_argument('-u', '--upper', **kw_max)
    parser.add_argument('-t', '--tzinfo', **kw_tz)
    args: ap.Namespace = parser.parse_args()
    return Config(args.lower, args.upper, args.tzinfo)


class Format:
    '''
    Constants of formatting exported data.

    Attributes:
        BPL (int): bytes per line
        LPA (int): lines per array for huge tables
        XX_BIT (int): bits per item of (NR, NY, YD, JS)
        XX_IPB (int): items per byte of (NR, NY, YD)
        JS_IPL (int): items per line of `cjie_to_usec`
    '''

    BPL: int = 12
    LPA: int = 3780
    NR_BIT, NR_IPB = 4, 2 # `nian` to `run`
    NY_BIT, NY_IPB = 1, 8 # `nian` to `cyue`
    YD_BIT, YD_IPB = 2, 4 # `cyue` to `uday`
    JS_BIT: int = 12 # `cjie` to `usec`
    JS_IPL: int = (BPL * 8) // JS_BIT


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


def load_data(csv_dir: str, conf: Config) -> Data:
    '''
    Loads lunar and solar data in bounds from CSVs.

    Args:
        csv_dir (str): directory containing CSV data files
        conf (Config): configurations of time bounds
    Returns:
        Data: lunar and solar data loaded from CSVs
    '''

    lo, hi, tz = conf
    # lunar: `nian` and `cyue`
    lu_path: str = os.path.join(csv_dir, 'lunar.csv')
    lu: pd.DataFrame = pd.read_csv(lu_path)
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
    # adds a new column `uday`
    SECS_PER_DAY: int = 86400
    tz_sec, _ = int_frac(tz * SECS_PER_DAY)
    lu['uday'] = (lu['usec'] + tz_sec) // SECS_PER_DAY
    # solar: `sui` and `jieqi`
    so_path: str = os.path.join(csv_dir, 'solar.csv')
    so: pd.DataFrame = pd.read_csv(so_path)
    so = so[so['sui'].between(lo, hi + 1)]
    return Data(lu, so)


# Part 1: Lunar Data on `riqi`


## Section 1.1. (`nian`, `ryue`) to `cyue` to `uday`


### 1.1.0. Preparations


class CoefsLevel(tp.NamedTuple):
    '''
    Fit form: `y = k0 * x + b0 + (k1 * x + b1 >> nb)`.

    Attributes:
        k0 (int): major linear
        b0 (int): major constant
        k1 (int): minor liear
        b1 (int): minor constant
        nb (int): bits shifted
    '''

    k0: int
    b0: int
    k1: int
    b1: int
    nb: int

    def predict(self: tp.Self, xs: Int64s) -> Int32s:
        '''
        Applies the fit form and coefficients.

        Assumptions:
            No overflows.

        Args:
            xs (Int32s): data input to fit
        Returns:
            Int32s: predicted values fitted
        '''

        major: Int64s = self.k0 * xs + self.b0
        minor: Int64s = self.k1 * xs + self.b1
        return major + (minor >> self.nb)


LevelCR: tp.TypeAlias = tuple[CoefsLevel, list[int]]


def level_fit(xs: pd.Series, ys: pd.Series, bit: int) -> LevelCR:
    '''
    Fits `ys` from `xs` like `CoefsLevel`.

    Args:
        xs (pd.Series): data input to fit
        ys (pd.Series): expected output of data
        bit (int): max bit length of one residual
    Returns:
        LevelCR: coefficients and residuals
    Raises:
        ValueError: if no feasible fit was found
    '''

    # fits like `y = k * x + b`
    kb: Floats = np.polyfit(xs, ys, deg=1)
    (k0, kf), (b0, bf) = map(int_frac, kb.tolist())
    # finds least feasible bits
    for nb in range(32):
        k1, _ = int_frac(kf * (1 << nb))
        b1, _ = int_frac(bf * (1 << nb))
        coefs: CoefsLevel = CoefsLevel(k0, b0, k1, b1, nb)
        xa: Int64s = xs.to_numpy().astype(np.int64)
        rs: pd.Series = ys - coefs.predict(xa)
        rmin: int = int(rs.min())
        coefs = CoefsLevel(k0, b0 + rmin, k1, b1, nb)
        if rs.max() - rmin < (1 << bit):
            return coefs, (rs - rmin).tolist()
    else:
        raise ValueError('bad idea, residuals too wide')


### 1.1.1. NR: `nian` to `run`: direct export


def nr_runs(lu: pd.DataFrame) -> list[int]:
    '''
    Gets `runyue` values of `nian`s in bounds.

    Args:
        lu (pd.DataFrame): lunar data in bounds
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


def ny_fit(lu: pd.DataFrame) -> LevelCR:
    '''
    Fits `nian_to_cyue` and gets residuals in `cyue`.

    Args:
        lu (pd.DataFrame): lunar data in bounds
    Returns:
        LevelCR: coefficients and residuals
    Raises:
        ValueError: if no feasible fit was found
    '''

    lo, hi = lu['nian'].min(), lu['nian'].max()
    num: int = int(hi) - int(lo) + 1
    NIAN_MOD: int = Format.NY_IPB * Format.BPL
    hi = lo + (num // NIAN_MOD) * NIAN_MOD - 1
    lu = lu[(lu['ryue'] == 2) & lu['nian'].between(lo, hi)]
    return level_fit(lu['nian'], lu['cyue'], Format.NY_BIT)


def ny_bytes(resy: list[int]) -> list[int]:
    '''
    Bitwise converts `nian_to_cyue` residuals to bytes.

    Args:
        resy (list[int]): NY residuals in `cyue`
    Returns:
        list[int]: bytes converted from NY residuals
    '''

    return vals_to_colls(resy, Format.NY_BIT, Format.NY_IPB)


### 1.1.3. YD: `cyue` to `uday`: linear (1-deg) regression


def yd_fit(lu: pd.DataFrame) -> LevelCR:
    '''
    Fits `cyue_to_uday` and gets residuals in uday.

    Args:
        lu (pd.DataFrame): lunar data in bounds
    Returns:
        LevelCR: coefficients and residuals
    Raises:
        ValueError: if no feasible fit was found
    '''

    lo, hi = lu['cyue'].min(), lu['cyue'].max()
    num: int = int(hi) - int(lo) + 1
    CYUE_NUM: int = Format.YD_IPB * Format.BPL
    hi = lo + (num // CYUE_NUM) * CYUE_NUM - 1
    lu = lu[lu['cyue'].between(lo, hi)]
    return level_fit(lu['cyue'], lu['uday'], Format.YD_BIT)


def yd_bytes(resd: list[int]) -> list[int]:
    '''
    Bitwise converts `cyue_to_uday` residuals to bytes.

    Args:
        resd (list[int]): YD residuals in `uday`
    Returns:
        list[int]: bytes converted from YD residuals
    '''

    return vals_to_colls(resd, Format.YD_BIT, Format.YD_IPB)


## Section 1.2. `uday` to `cyue` to `nian`, no residuals


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
        '''
        Applies the fit form and coefficients.

        Assumptions:
            No overflows.

        Args:
            xs (Int64s): data input to fit
        Returns:
            Int64s: predicted values fitted
        '''

        b0, k1, b1, nb = self
        return b0 + (k1 * xs + b1 >> nb)


def exact_fit(xs: pd.Series, ys: pd.Series) -> CoefsExact:
    '''
    Fits `ys` from `xs` like `CoefsExact`.

    Args:
        xs (pd.Series): data input to fit
        ys (pd.Series): expected output of data
    Returns:
        CoefsExact: coefficients for fitting
    Raises:
        ValueError: if no feasible fit was found

    Notes:
        * Residuals are NOT allowed here.
    '''

    # fits like `y = k * x + b`
    k, b = np.polyfit(xs, ys, deg=1)
    b0, bf = int_frac(float(b))
    # finds least feasible bits
    for nb in range(32):
        k1, _ = int_frac(float(k) * (1 << nb))
        b1, _ = int_frac(bf * (1 << nb))
        coefs: CoefsExact = CoefsExact(b0, k1, b1, nb)
        xa: Int64s = xs.to_numpy().astype(np.int64)
        if np.all(ys == coefs.predict(xa)):
            return coefs
    else:
        raise ValueError('bad idea, residuals too wide')


### 1.2.1. DY: `uday` to `cyue`
### TODO: complete it some daytime; sleepy


def main() -> None:
    conf: Config = load_config()
    here: str = os.path.dirname(__file__)
    csv_dir: str = os.path.join(here, 'build')
    lu, so = load_data(csv_dir, conf)
    nr_rs: list[int] = nr_runs(lu)
    nr_bt: list[int] = nr_bytes(nr_rs)
    ny_cf, ny_ry = ny_fit(lu)
    ny_bt: list[int] = ny_bytes(ny_ry)
    yd_cf, yd_rd = yd_fit(lu)
    yd_bt: list[int] = yd_bytes(yd_rd)


if __name__ == '__main__':
    main()