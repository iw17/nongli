# %% [markdown]
# # Coefficients & Residuals

# %% [markdown]
# ## 0. Meta

# %%
import ctypes
import os
import time
import typing as tp

# %%
import numpy as np
import pandas as pd
from numpy.typing import NDArray

# %%
UInt8s: tp.TypeAlias = NDArray[np.uint8]
Int64s: tp.TypeAlias = NDArray[np.int64]
Floats: tp.TypeAlias = NDArray[np.float64]

# %%
PTR: int = ctypes.sizeof(ctypes.c_void_p)
print(f'{PTR} bytes in a pointer')

# %%
MIN: int = +1900
MAX: int = +2199
BPL: int = 12 # bytes per line
LPA: int = 315 # lines per array

# %%
t_begin: float = time.perf_counter()
here: str = os.path.abspath('')
csv_dir: str = os.path.join(here, 'build')
lu_path: str = os.path.join(csv_dir, 'lunar.csv')
so_path: str = os.path.join(csv_dir, 'solar.csv')
hpp_dir: str = os.path.join(here, '..', 'fit')
ex_path: str = os.path.join(hpp_dir, 'data.hpp')

# %%
def int_frac(x: float) -> tuple[int, float]:
    '''
    Splits a float to integral and fractional parts.

    Args:
        x (float): input floating-point value
    Returns:
        int: the maximal integer `i <= x`
        float: `x - i` in [0, 1)
    '''

    xi, xf = divmod(x, 1)
    return int(xi), xf

# %% [markdown]
# ## 1. Lunar

# %% [markdown]
# ### 1.0. Data

# %%
lu_df: pd.DataFrame = pd.read_csv(lu_path)
lu_va: pd.Series = lu_df['nian'].between(MIN, MAX)
cyue: pd.Series = lu_df['cyue']
lu_df['uday'] = (lu_df['usec'] + 28800) // 86400
uday: pd.Series = lu_df['uday']
lu_df.head()

# %% [markdown]
# ### 1.1. N2G: (`nian`, `ryue`) to `cyue` to `uday`

# %% [markdown]
# #### 1.1.1. `nian` to `run`

# %%
nr_ryue: pd.Series = lu_df['ryue']
nr_rn: pd.Series = (lu_df['ryue'] & 1).astype(np.bool)
nr_nian: pd.Series = lu_df['nian'][nr_rn]
nr_ry: pd.Series = nr_ryue[nr_rn] // 2
nr_dc: dict[int, int] = dict(zip(nr_nian, nr_ry))
print(f'{len(nr_dc)} runs')

# %%
NR_BIT, NR_IPB = 4, 2
NR_IPL: int = NR_IPB * BPL
nr_cnt_va: int = (MAX - MIN + 1) + (MIN - MAX - 1) % NR_IPL
nr_ry_bt: list[int] = []
for i in range(0, nr_cnt_va, NR_IPB):
    bt: int = 0
    for j in range(NR_IPB):
        n: int = MIN + i + j
        bt |= nr_dc.get(n, 13) << (j * NR_BIT)
    nr_ry_bt.append(bt)
nr_ry_np: UInt8s = np.array(nr_ry_bt, dtype=np.uint8)
nr_ry_np = nr_ry_np.reshape((-1, BPL))
nr_ry_ln: list[list[int]] = nr_ry_np.tolist()
print(f'{len(nr_ry_bt)} bytes')

# %% [markdown]
# #### 1.1.2. `nian` to `cyue`: linear (1-deg) regression

# %%
ny_va: pd.Series = lu_df['ryue'] == 2
ny_nian: pd.Series = lu_df['nian'][ny_va]
ny_dp01: pd.Series = lu_df['cyue'][ny_va]
ny_coef: Floats = np.polyfit(x=ny_nian, y=ny_dp01, deg=1)
ny_c1, ny_c0 = tp.cast(list[float], ny_coef.tolist())
print(f'cyue = {ny_c1} * nian + {ny_c0}')

# %%
ny_c1i, ny_c1f = int_frac(ny_c1)
ny_c0i, ny_c0f = int_frac(ny_c0)
print(ny_c1i, ny_c1f, ny_c0i, ny_c0f)

# %%
NY_BIT, NY_IPB = 1, 8
for ny_bits in range(32):
    ny_c0ib: int = ny_c0i
    ny_c1bi, _ = int_frac(ny_c1f * (1 << ny_bits))
    ny_c0bi, _ = int_frac(ny_c0f * (1 << ny_bits))
    ny_pred: pd.Series = ny_c1i * ny_nian + ny_c0i
    ny_bias: pd.Series = ny_c1bi * ny_nian + ny_c0bi
    ny_pred += ny_bias // (1 << ny_bits)
    ny_resy: pd.Series = ny_dp01 - ny_pred
    if ny_resy.max() - ny_resy.min() < (1 << NY_BIT):
        ny_c0ib += ny_resy.min()
        ny_resy -= ny_resy.min()
        print(f'cyue = {ny_c1i} * nian + {ny_c0ib}', end=' + ')
        print(f'({ny_c1bi} * nian + {ny_c0bi} >> {ny_bits})')
        print(f'resy in {range(ny_resy.min(), ny_resy.max() + 1)}')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %%
NY_IPL: int = NY_IPB * BPL
ny_va: pd.Series = ny_nian.between(MIN, MAX)
ny_cnt_va: int = ny_va.sum() + 1 + (-(ny_va.sum() + 1) % NY_IPL)
ny_resy_va: pd.Series = ny_resy[ny_nian >= MIN].iloc[:ny_cnt_va]
ny_resy_bt: list[int] = []
for i in range(0, len(ny_resy_va), NY_IPB):
    bt: int = 0
    for j, r in enumerate(ny_resy_va.iloc[i:i+NY_IPB]):
        bt |= r << (j * NY_BIT)
    ny_resy_bt.append(bt)
ny_resy_np: UInt8s = np.array(ny_resy_bt, dtype=np.uint8)
ny_resy_np = ny_resy_np.reshape((-1, BPL))
ny_resy_ln: list[list[int]] = ny_resy_np.tolist()
print(f'{len(ny_resy_bt)} bytes')

# %% [markdown]
# #### 1.1.3. `cyue` to `uday`: linear (1-deg) regression

# %%
yd_coef: Floats = np.polyfit(x=cyue, y=uday, deg=1)
yd_c1, yd_c0 = tp.cast(list[float], yd_coef.tolist())
print(f'uday = {yd_c1} * cyue + {yd_c0}')

# %%
yd_c1i, yd_c1f = int_frac(yd_c1)
yd_c0i, yd_c0f = int_frac(yd_c0)
print(yd_c1i, yd_c1f, yd_c0i, yd_c0f)

# %%
YD_BIT, YD_IPB = 2, 4
for yd_bits in range(64):
    yd_c0ib: int = yd_c0i
    yd_c1bi, _ = int_frac(yd_c1f * (1 << yd_bits))
    yd_c0bi, _ = int_frac(yd_c0f * (1 << yd_bits))
    yd_pred: pd.Series = yd_c1i * cyue + yd_c0i
    yd_bias: pd.Series = yd_c1bi * cyue + yd_c0bi
    yd_pred += yd_bias // (1 << yd_bits)
    yd_resd: pd.Series = uday - yd_pred
    if yd_resd.max() - yd_resd.min() < 1 << YD_BIT:
        yd_c0ib += yd_resd.min()
        yd_resd -= yd_resd.min()
        print(f'uday = {yd_c1i} * cyue + {yd_c0ib}', end=' + ')
        print(f'({yd_c1bi} * cyue + {yd_c0bi} >> {yd_bits})')
        print(f'resd in {range(yd_resd.min(), yd_resd.max() + 1)}')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %%
YD_IPL: int = YD_IPB * BPL
yd_cnt_va: int = lu_va.sum() + 1 + (-(lu_va.sum() + 1) % YD_IPL)
yd_va: pd.Series = lu_df['nian'] >= MIN
yd_resd_va: pd.Series = yd_resd[yd_va].iloc[:yd_cnt_va]
yd_resd_bt: list[int] = []
for i in range(0, len(yd_resd_va), YD_IPB):
    bt: int = 0
    for j, r in enumerate(yd_resd_va.iloc[i:i+YD_IPB]):
        bt |= r << (j * YD_BIT)
    yd_resd_bt.append(bt)
yd_resd_np: UInt8s = np.array(yd_resd_bt, dtype=np.uint8)
yd_resd_np = yd_resd_np.reshape((-1, BPL))
yd_resd_ln: list[list[int]] = []
for i in range(0, yd_resd_np.shape[0], LPA):
    yd_resd_ln.append(yd_resd_np[i:i+LPA].tolist())
print(f'{len(yd_resd_bt)} bytes')

# %% [markdown]
# ### 1.2. G2N: `uday` to `cyue` to (`nian`, `ryue`)

# %% [markdown]
# #### 1.2.1. `uday` to `cyue`: linear (1-deg) regression

# %%
dy_coef: Floats = np.polyfit(x=uday, y=cyue, deg=1)
dy_c1, dy_c0 = tp.cast(list[float], dy_coef.tolist())
dy_c0 += 0.5 # it is midpoint that is to be predicted
print(f'cyue = {dy_c1} * cday + {dy_c0}')

# %%
_, dy_c1f = int_frac(dy_c1)
dy_c0i, dy_c0f = int_frac(dy_c0)
print(dy_c1f, dy_c0i, dy_c0f)

# %%
for dy_bits in range(64):
    dy_c1bi, _ = int_frac(dy_c1f * (1 << dy_bits))
    dy_c0bi, _ = int_frac(dy_c0f * (1 << dy_bits))
    dy_bias: pd.Series = dy_c1bi * uday + dy_c0bi
    dy_pred: pd.Series = dy_c0i + dy_bias // (1 << dy_bits)
    if np.all(dy_pred == cyue):
        print(f'cyue = {dy_c0i}', end=' + ')
        print(f'({dy_c1bi} * uday + {dy_c0bi} >> {dy_bits})')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %% [markdown]
# #### 1.2.2. `cyue` to `nian`: linear (1-deg) regression

# %%
yn_cyue: pd.Series = lu_df['cyue'][lu_df['ryue'] == 2]
yn_nian: pd.Series = lu_df['nian'][lu_df['ryue'] == 2]
yn_coef: Floats = np.polyfit(x=yn_cyue, y=yn_nian, deg=1)
yn_c1, yn_c0 = tp.cast(list[float], yn_coef.tolist())
yn_c0 += 0.5 # similarly, midpoints are considered
print(f'nian = {yn_c1} * cyue + {yn_c0}')

# %%
_, yn_c1f = int_frac(yn_c1)
yn_c0i, yn_c0f = int_frac(yn_c0)
print(yn_c1f, yn_c0i, yn_c0f)

# %%
for yn_bits in range(64):
    yn_c1bi, _ = int_frac(yn_c1f * (1 << yn_bits))
    yn_c0bi, _ = int_frac(yn_c0f * (1 << yn_bits))
    yn_bias: pd.Series = yn_c1bi * yn_cyue + yn_c0bi
    yn_pred: pd.Series = yn_c0i + yn_bias // (1 << yn_bits)
    if np.all(yn_pred == yn_nian):
        print(f'nian = {yn_c0i}', end=' + ')
        print(f'({yn_c1bi} * cyue + {yn_c0bi} >> {yn_bits})')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %% [markdown]
# ## 2. Solar

# %% [markdown]
# ### 2.0. Data

# %%
so_df: pd.DataFrame = pd.read_csv(so_path)
cjie: pd.Series = so_df['cjie']
suis: pd.Series = so_df['sui']
jies: pd.Series = so_df['jie']
usec: pd.Series = so_df['usec']
so_va: pd.Series = suis.between(MIN, MAX)
so_df.head()

# %% [markdown]
# ### 2.1. `cjie` to `usec`: sextic (6-deg) regression

# %%
js_coef_ls: list[Floats] = []
for jie in range(24):
    js_gr_suis: pd.Series = suis[jies == jie]
    js_gr_usec: pd.Series = usec[jies == jie]
    js_coef_gr: Floats = np.polyfit(x=js_gr_suis, y=js_gr_usec, deg=6)
    js_coef_ls.append(js_coef_gr)
js_coef_np: Floats = np.array(js_coef_ls, dtype=np.float64)
js_c0_np: Floats = js_coef_np[:, -1]
js_c0m, _ = int_frac(js_c0_np.min().item())
js_coef_np[:, -1] -= js_c0m
js_c1_np: Floats = js_coef_np[:, -2]
js_c1m, _ = int_frac(js_c1_np.min().item())
js_coef_np[:, -2] -= js_c1m
print(f'coefficient matrix shape {js_coef_np.shape}')

# %%
JS_BIT: int = 12
js_va: pd.Series = suis >= MIN
for js_bits in range(32):
    js_shls: Floats = 2.0 ** (js_bits * np.arange(7)[::-1]) # type: ignore
    js_cshl_np: Floats = js_coef_np * js_shls
    js_ci_np: Int64s = np.round(js_cshl_np).astype(np.int64)
    js_rest_ls: list[list[int]] = []
    js_ress_ext_ls: list[int] = []
    for jie in range(24):
        js_gr_suis: pd.Series = suis[jies == jie]
        js_gr_usec: pd.Series = usec[jies == jie]
        js_gr_c0, *su_gr_c1r = tp.cast(list[int], js_ci_np[jie].tolist())
        js_gr_pr: pd.Series = 0 * js_gr_suis + js_gr_c0
        for ci in su_gr_c1r:
            js_gr_pr = js_gr_pr * js_gr_suis // (1 << js_bits) + ci
        js_gr_pr += js_c1m * js_gr_suis + js_c0m
        js_gr_ress = js_gr_usec - js_gr_pr
        js_rest_ls.append(js_gr_ress[js_va].to_list())
        js_ress_ext_ls.append(js_gr_ress.max() - js_gr_ress.min())
    if max(js_ress_ext_ls) < 1 << JS_BIT:
        js_resm_ls: list[list[int]] = list(zip(*js_rest_ls)) # type: ignore
        js_resm_np: Int64s = np.array(js_resm_ls, dtype=np.int64)
        js_resm_min: Int64s = js_resm_np.min(axis=0)
        js_ci_np[:, -1] += 2 * (1 << JS_BIT) + js_resm_min
        js_c0mb: int = js_c0m - 2 * (1 << JS_BIT)
        print(f'usec = {js_c1m} * sui + {js_c0mb} + ({js_bits} bits/deg)')
        js_resm_np -= js_resm_min
        print(f'ress in {range(js_resm_np.min(), js_resm_np.max() + 1)}')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %%
JS_IPL: int = JS_BIT * 8 // BPL
js_cnt_va: int = so_va.sum() + 1 + (-so_va.sum() - 1) % JS_IPL
js_ress_va: list[int] = js_resm_np.ravel()[:js_cnt_va].tolist()
js_ress_bt: list[int] = []
# with JS_BIT == 12 == 1.5 bytes
for old, new in zip(js_ress_va[0::2], js_ress_va[1::2]):
    lo: int = old & 0xff
    md: int = (old >> 8) | ((new & 0x0f) << 4)
    hi: int = new >> 4
    js_ress_bt.extend([lo, md, hi])
js_ress_np: UInt8s = np.array(js_ress_bt, dtype=np.uint8)
js_ress_np = js_ress_np.reshape((-1, BPL))
js_ress_ln: list[list[int]] = []
for i in range(0, js_ress_np.shape[0], LPA):
    js_ress_ln.append(js_ress_np[i:i+LPA].tolist())
print(f'{len(js_ress_bt)} bytes')

# %% [markdown]
# ### 2.2. `usec` to `cjie`: linear (1-deg) regression

# %%
sj_coef: Floats = np.polyfit(x=usec, y=cjie, deg=1)
sj_c1, sj_c0 = tp.cast(list[float], sj_coef.tolist())
sj_c0 += 0.5
print(f'cjie = {sj_c1} * usec + {sj_c0}')

# %%
_, sj_c1f = int_frac(sj_c1)
sj_c0i, sj_c0f = int_frac(sj_c0)
print(sj_c1f, sj_c0i, sj_c0f)

# %%
for sj_bits in range(32):
    sj_c1bi, _ = int_frac(sj_c1f * (1 << (2 * sj_bits)))
    sj_c0bi, _ = int_frac(sj_c0f * (1 << sj_bits))
    sj_pred: pd.Series = (sj_c1bi * usec) // (1 << sj_bits)
    sj_pred = (sj_pred + sj_c0bi) // (1 << sj_bits) + sj_c0i
    if np.all(sj_pred == cjie):
        print(f'cjie = {sj_c0i}', end=' + ')
        print(f'(({sj_c1bi} * usec >> {sj_bits})', end=' + ')
        print(f'{sj_c0bi} >> {sj_bits})')
        break
else:
    raise ValueError('bad idea, residuals too wide')

# %% [markdown]
# ## 3. Export

# %% [markdown]
# ### 3.0. Bounds & Formats

# %%
VPL: int = 5 # variables per line
ICX: str = 'inline constexpr'
os.makedirs(hpp_dir, exist_ok=True)
with open(ex_path, 'w') as ex_hpp:
    pass # clear `data.cpp` if it exists
ex_fmt_bt: str = '   ' + BPL * ' 0x{:02x},' + '\n'
print(ex_fmt_bt[:60])

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write('#ifndef IW_DATA_HPP\n')
    ex_hpp.write('#define IW_DATA_HPP 20250203L\n\n')
    ex_hpp.write('#include <cstdint>\n\n')
    ex_hpp.write('namespace iw17::_data {\n\n')
    ex_hpp.write(f'{ICX} int16_t NIAN_MIN = {MIN:+d};\n')
    ex_hpp.write(f'{ICX} int16_t NIAN_MAX = {MAX:+d};\n\n')
    ex_hpp.write(f'{ICX} int16_t SUI_MIN = {MIN:+d};\n')
    ex_hpp.write(f'{ICX} int16_t SUI_MAX = {MAX:+d};\n\n')
    ex_hpp.write(f'{ICX} int16_t YEAR_MIN = {MIN:+d};\n')
    ex_hpp.write(f'{ICX} int16_t YEAR_MAX = {MAX:+d};\n\n')
    ex_cyue_min: int = cyue[lu_va].min()
    ex_cyue_max: int = cyue[lu_va].max()
    ex_cyue_num: int = ex_cyue_max - ex_cyue_min + 1
    ex_hpp.write(f'{ICX} int32_t CYUE_MIN = {ex_cyue_min:+d};\n')
    ex_hpp.write(f'{ICX} int32_t CYUE_MAX = {ex_cyue_max:+d};\n\n')
    ex_cjie_min: int = cjie[so_va].min()
    ex_cjie_max: int = cjie[so_va].max()
    ex_cjie_num: int = ex_cjie_max - ex_cjie_min + 1
    ex_hpp.write(f'{ICX} int32_t CJIE_MIN = {ex_cjie_min:+d};\n')
    ex_hpp.write(f'{ICX} int32_t CJIE_MAX = {ex_cjie_max:+d};\n\n')
ex_bt_meta: int = 2 * 6 + 4 * 4
print(f'{ex_bt_meta} bytes for bounds')

# %% [markdown]
# ### 3.1. Lunar

# %% [markdown]
# #### 3.1.0. Formats

# %%
ex_fl_hi: str = ', '.join(4 * ['{:d}'])
ex_fl_wl: list[int] = np.char.str_len(np.array([
    [dy_c0i, dy_c1bi, dy_c0bi],
    [yn_c0i, yn_c1bi, yn_c0bi],
], dtype=str)).max(axis=0).tolist()
ex_fl_lo: str = ', '.join(3 * ['{:d}'])
print(ex_fl_hi)
print(ex_fl_lo)

# %% [markdown]
# #### 3.1.1. `nian` to `run`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} uint8_t NR_RUNS[] = {{\n')
    for ex_nr_ln in nr_ry_ln:
        ex_hpp.write(ex_fmt_bt.format(*ex_nr_ln))
    ex_hpp.write('};\n\n')
ex_bt_nr: int = 1 * len(nr_ry_bt)
print(f'{ex_bt_nr} bytes for NR runs')

# %% [markdown]
# #### 3.1.2. `nian` to `cyue`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t NY_BITS = {ny_bits};\n\n')
ex_bt_ny_bits: int = 8 # 1 saves nothing due to alignment
print(f'{ex_bt_ny_bits} bytes for NY bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t NY_COEF[] = {{ ')
    ex_hpp.write(ex_fl_hi.format(ny_c1i, ny_c0ib, ny_c1bi, ny_c0bi))
    ex_hpp.write(' };\n\n')
ex_bt_ny_coef: int = 8 * 4
print(f'{ex_bt_ny_coef} bytes for NY coef')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} uint8_t NY_RESY[] = {{\n')
    for ex_ny_ln in ny_resy_ln:
        ex_hpp.write(ex_fmt_bt.format(*ex_ny_ln))
    ex_hpp.write('};\n\n')
ex_bt_ny_resy: int = 1 * len(ny_resy_bt)
print(f'{ex_bt_ny_resy} bytes for NY resd')

# %% [markdown]
# #### 3.1.3. `cyue` to `uday`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t YD_BITS = {yd_bits};\n\n')
ex_bt_yd_bits: int = 8
print(f'{ex_bt_yd_bits} bytes for YD bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t YD_COEF[] = {{ ')
    ex_hpp.write(ex_fl_hi.format(yd_c1i, yd_c0ib, yd_c1bi, yd_c0bi))
    ex_hpp.write(' };\n\n')
ex_bt_yd_coef: int = 8 * 4
print(f'{ex_bt_yd_coef} bytes for YD coef')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_yd_na: int = len(yd_resd_ln)
    ex_yd_nw: int = len(str(ex_yd_na))
    ex_yd_ns: list[str] = [
        f'YD_RESD_{i:0{ex_yd_nw}d}' for i in range(ex_yd_na)
    ]
    ex_hpp.write(f'{ICX} int64_t YD_PAGE = {BPL * LPA};\n\n')
    for ex_yd_nv, ex_yd_ap in zip(ex_yd_ns, yd_resd_ln):
        ex_hpp.write(f'{ICX} uint8_t {ex_yd_nv}[] = {{\n')
        for ex_yd_ln in ex_yd_ap:
            ex_hpp.write(ex_fmt_bt.format(*ex_yd_ln)) # type: ignore
        ex_hpp.write('};\n\n')
    ex_hpp.write(f'{ICX} const uint8_t *YD_ARRD[] = {{\n')
    for i in range(0, ex_yd_na, VPL):
        ex_yd_al: list[str] = ex_yd_ns[i:i+VPL]
        ex_yd_ll: int = len(ex_yd_al)
        ex_yd_fmt: str = '   ' + ex_yd_ll * ' {},' + '\n'
        ex_hpp.write(ex_yd_fmt.format(*ex_yd_al))
    ex_hpp.write('};\n\n')
ex_bt_yd_resd: int = 8 + 1 * len(yd_resd_bt) + PTR * ex_yd_na
print(f'{ex_bt_yd_resd} bytes for YD resd')

# %% [markdown]
# #### 3.1.4. `uday` to `cyue`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t DY_BITS = {dy_bits};\n\n')
ex_bt_dy_bits: int = 8
print(f'{ex_bt_dy_bits} bytes for DY bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t DY_COEF[] = {{ ')
    ex_hpp.write(ex_fl_lo.format(dy_c0i, dy_c1bi, dy_c0bi))
    ex_hpp.write(' };\n\n')
ex_bt_dy_coef: int = 8 * 3
print(f'{ex_bt_dy_coef} bytes for DY coef')

# %% [markdown]
# #### 3.1.5. `cyue` to `nian`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t YN_BITS = {yn_bits};\n\n')
ex_bt_yn_bits: int = 8
print(f'{ex_bt_yn_bits} bytes for YN bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t YN_COEF[] = {{ ')
    ex_hpp.write(ex_fl_lo.format(yn_c0i, yn_c1bi, yn_c0bi))
    ex_hpp.write(' };\n\n')
ex_bt_yn_coef: int = 8 * 3
print(f'{ex_bt_yn_coef} bytes for YN coef')

# %% [markdown]
# ### 3.2. Solar

# %% [markdown]
# #### 3.2.0. Formats

# %%
ex_fs_wh: Int64s = np.char.str_len(js_ci_np.astype(str)).max(axis=0)
ex_fs_hi: str = ', '.join(7 * ['{{:{}d}}']).format(*ex_fs_wh)
ex_fs_lo: str = ', '.join(3 * ['{:d}'])
print(ex_fs_hi)
print(ex_fs_lo)

# %% [markdown]
# #### 3.2.1. `sui` to `usec`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t JS_BITS = {js_bits};\n\n')
ex_bt_js_bits: int = 8
print(f'{ex_bt_js_bits} bytes for JS bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t JS_CLIN[] = ')
    ex_hpp.write(f'{{ {js_c1m}, {js_c0mb} }};\n\n')
ex_bt_js_clin: int = 8 * 2
print(f'{ex_bt_js_clin} bytes for JS clin')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t JS_COEF[][7] = {{\n')
    for ex_js_ci in tp.cast(list[int], js_ci_np.tolist()):
        ex_js_ci_ln: str = ex_fs_hi.format(*ex_js_ci) # type: ignore
        ex_hpp.write(f'    {{ {ex_js_ci_ln} }},\n')
    ex_hpp.write('};\n\n')
ex_bt_js_coef: int = 8 * 7 * 24
print(f'{ex_bt_js_coef} bytes for JS coef')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_js_na: int = len(js_ress_ln)
    ex_js_nw: int = len(str(ex_js_na))
    ex_js_ns: list[str] = [
        f'JS_RESS_{i:0{ex_js_nw}d}' for i in range(ex_js_na)
    ]
    ex_hpp.write(f'{ICX} int64_t JS_PAGE = {BPL * LPA};\n\n')
    for ex_js_nv, ex_js_ap in zip(ex_js_ns, js_ress_ln):
        ex_hpp.write(f'{ICX} uint8_t {ex_js_nv}[] = {{\n')
        for ex_js_ln in ex_js_ap:
            ex_hpp.write(ex_fmt_bt.format(*ex_js_ln)) # type: ignore
        ex_hpp.write('};\n\n')
    ex_hpp.write(f'{ICX} const uint8_t *JS_ARRS[] = {{\n')
    for i in range(0, ex_js_na, VPL):
        ex_js_al: list[str] = ex_js_ns[i:i+VPL]
        ex_js_ll: int = len(ex_js_al)
        ex_js_fmt: str = '   ' + ex_js_ll * ' {},' + '\n'
        ex_hpp.write(ex_js_fmt.format(*ex_js_al))
    ex_hpp.write('};\n\n')
ex_bt_js_resd: int = 8 + 1 * len(js_ress_bt) + PTR * ex_js_na
print(f'{ex_bt_js_resd} bytes for JS ress')

# %% [markdown]
# #### 3.2.2. `usec` to `cjie`

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t SJ_BITS = {sj_bits};\n\n')
ex_bt_sj_bits: int = 8
print(f'{ex_bt_sj_bits} bytes for SJ bits')

# %%
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write(f'{ICX} int64_t SJ_COEF[] = {{ ')
    ex_hpp.write(ex_fs_lo.format(sj_c0i, sj_c1bi, sj_c0bi))
    ex_hpp.write(' };\n\n')
ex_bt_sj_coef: int = 8 * 3
print(f'{ex_bt_sj_coef} bytes for SJ coef')

# %% [markdown]
# ## 4. Statistics

# %%
ex_bt: int = sum([eval(v) for v in dir() if v.startswith('ex_bt_')])
with open(ex_path, 'a') as ex_hpp:
    ex_hpp.write('} // namespace iw17::_data, ')
    ex_hpp.write(f'{ex_bt} bytes in total\n\n')
    ex_hpp.write('#endif // IW_DATA_HPP\n\n')
print(f'{ex_bt} bytes exported as data')
t_end: float = time.perf_counter()
sec_taken: float = t_end - t_begin
print(f'{sec_taken:.3f} s taken for data generation')

# %% [markdown]
# ## 5. Vocabulary

# %% [markdown]
# Lunar:
# * NR: `nian` to `run`
# * NY: `nian` to `cyue`
# * YD: `cyue` to `uday`
# * DY: `uday` to `cyue`
# * YN: `cyue` to `nian`
# 
# Solar:
# * JS: `cjie` to `usec`
# * SJ: `usec` to `cjie`
# 
# P.S.
# * A new `nongli` day comes at 00:00:00 UTC+8,
#   and DST from 1985 to 1991 is taken out of account.
# * 1970 `nian` comes on `chunjie` (p01-01), while
#   1970 `sui` comes at `dongzhi` (1969-12-22 08:43:41 UTC+8).
# * The prefix `u` in `uday` and `usec` means Unix,
#   counting from Unix Epoch (1970-01-01 00:00:00 UTC).
# * The prefix `c` in `cyue` and `cjie` means cumulative,
#   counting from 1970-p01 and 1970 `dongzhi` resp.
# * `Nongli` has both lunar and solar. NEVER refer to
#   CHINESE new year `chunjie` as LUNAR new year!


