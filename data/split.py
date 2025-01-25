'''
Splits raw data file into lunar and solar components.
Note: Please reopen with UTF-8 if it is messed up.
'''

import io
import os
import re
import sys
import typing as tp

sys.path.append(os.path.dirname(__file__))
import dati as dt
import hanzi as hz


def parse_year(line: str) -> int:
    '''
    Parses the line string as a Gregorian year notation.

    Args:
        line (str): string noted with a year number
    Returns:
        int: the Gregorian year it represents
    Raises:
        ValueError: when no digit is in the input

    Examples:
        >>> parse_year('2024')
        2024
        >>> parse_year('-840tail')
        -840
        >>> parse_year('B4713')
        -4712
        >>> parse_year('text')
        ValueError: 'text' has no digit
    '''

    match: re.Match[str] | None = re.search(r'-?\d+', line)
    if match is None:
        raise ValueError(f'\'{line}\' has no digit')
    year: int = int(match.group(0))
    return 1 - year if line.startswith('B') else year


class Item(tp.NamedTuple):
    '''
    Datetime item without year.

    Attributes:
        m (int): month, from 1 to 12
        d (int): day, from 1 to 31
        hh (int): hour, from 0 to 23
        mm (int): minute, from 0 to 59
        ss (int): second, from 0 to 60
    '''

    m: int
    d: int
    hh: int
    mm: int
    ss: int


def parse_item(line: str) -> Item:
    '''
    Parses the line string and re-format the datetime.

    Args:
        line (str): item string 'MM-DD(dd hh:mm:ss)'
    Returns:
        list[int]: parsed integer values [M, d, h, m, s]
    Raises:
        ValueError: when line does not contain exactly 6 numbers

    Examples:
        >>> parse_item('02-04(04 16:27:05)')
        Item(m=2, d=4, hh=16, mm=27, ss=5)
        >>> parse_item('02-01(31 16:17:47)')
        Item(m=1, d=31, hh=16, mm=17, ss=47)
        >>> parse_item('10-31(02 00:07:34)')
        Item(m=11, d=2, hh=0, mm=7, ss=34)
        >>> parse_item('1 2 3 4 5 6')
        Item(m=1, d=3, hh=4, mm=5, ss=6)
        >>> parse_item('1 2 3 4 5')
        ValueError: not enough values to unpack (expected 6, got 5)
        >>> parse_item('1 2 3 4 5 6 7')
        ValueError: too many values to unpack (expected 6)
    '''

    THRESHOLD: int = 16
    mon, rd, pd, hh, mm, ss = map(int, re.findall(r'\d+', line))
    mon += int((rd - pd) / THRESHOLD)
    item = Item(mon, pd, hh, mm, ss)
    return item


def del_repeat(it: list[tp.Any], k: int) -> list[tp.Any]:
    '''
    Deletes those repeated in recent `k` entries.

    Args:
        it (list[Any]): list to be filtered
        k (int): how many entries to look back at
    Returns:
        list[Any]: the non-`k`-recently repeated entries

    Examples:
        >>> del_repeat([6, 5, 9, 9, 2, 0, 5, 8], k=1)
        [6, 5, 9, 2, 0, 5, 8]
        >>> del_repeat([6, 5, 9, 9, 2, 0, 5, 8], k=4)
        [6, 5, 9, 2, 0, 8]
    '''

    if k <= 0:
        raise ValueError(f'invalid non-positive {k} as k')
    non_repeated: list[tp.Any] = []
    for entry in it:
        if entry not in non_repeated[-k:]:
            non_repeated.append(entry)
    return non_repeated


def add_years(items: list[Item], year: int) -> list[dt.Dati]:
    '''
    Adds year prefixes to the list of dati items.

    Assumptions:
        * `items` has been `del_repeat`ed.

    Args:
        items (list[Item]): list of dati items without years
        year (int): start year of the dati items, increasing
    Returns:
        list[dt.Dati]: the corresponding datis with years
    '''

    datis: list[dt.Dati] = []
    for old_item, new_item in zip(items[:-1], items[1:]):
        datis.append(dt.Dati(year, *old_item))
        if new_item < old_item:
            year += 1
    else:
        datis.append(dt.Dati(year, *items[-1]))
    return datis


def bin_search(seq: tp.Sequence[tp.Any], val: tp.Any) -> int:
    '''
    Binary searches `seq` for `val`.
    Note: `val` does NOT have to be in `seq`.

    Assumptions:
        * `seq` is ascendingly totally ordered.
        * `seq` contains each entry only once.
        * `val` is comparable with entries in `seq`.

    Args:
        seq (Sequence[Any]): list to be searched
        val (Any): entry to be searched for
    Returns:
        int: index of the last `seq` entry <= `val`

    Examples:
        >>> bin_search(seq='127', val='0')
        -1
        >>> bin_search(seq='127', val='1')
        0
        >>> bin_search(seq='127', val='4')
        1
        >>> bin_search(seq='127', val='8')
        2
    '''

    lo, hi = 0, len(seq) - 1
    while True:
        # boundary
        if val < seq[lo]:
            return lo - 1
        if val >= seq[hi]:
            return hi
        if hi - lo <= 1:
            return hi if val >= seq[hi] else lo
        md: int = (lo + hi) // 2
        if val == seq[md]:
            return md
        # increment
        lo, hi = (lo, md - 1) if val < seq[md] else (md, hi)


class Split:
    '''
    Splitter of raw data to lunar and solar lists.

    Assumptions:
        * `item_lu` starts incl. and ends excl. with `p11`.
        * `item_so` starts with `dongzhi`, `daxue` or `xiaoxue`.

    Attributes:
        lead_yr (int): leading year of the lunar and solar items
        lead_jq (int): leading `jieqi` of the solar items
        last_dy (bool): whether the last lunar item is a `dayue`
        dati_lu (list[Dati]): `shuo` datis with year prefixes
        dati_so (list[Dati]): `jieqi` datis with year prefixes
        runs_lu (list[bool]): whether each `yue` is `run`
    '''


    def _init_datis(self: tp.Self, in_fp: io.TextIOBase) -> None:
        '''
        Initializes lunar and solar datis and boundary values.

        Assumptions:
            * The data text in `in_fp` is well-formatted.
            * `in_fp` starts incl. and ends excl. with `p11`.

        Adds:
            lead_yr (int): leading year of the lunar and solar items
            lead_jq (int): leading `jieqi` of the solar items
            last_dy (bool): whether the last lunar item is a `dayue`
            dati_lu (list[Dati]): `shuo` datis with year prefixes
            dati_so (list[Dati]): `jieqi` datis with year prefixes
        '''

        line: str = in_fp.readline().strip()
        self.lead_yr: int = parse_year(line) # leading year
        # leading item line
        line = in_fp.readline().strip()
        item_lu: list[Item] = []
        item_so: list[Item] = []
        SEP: str = ' ' * 4
        shuo, *jieqis = line.split(SEP)
        info, item = shuo.split(maxsplit=1)
        self.lead_jq: int = hz.solar_index(jieqis[0][:2])
        item_lu.append(parse_item(item))
        item_so.extend(map(parse_item, jieqis))
        # other item lines
        while (line := in_fp.readline().strip()):
            if SEP not in line:
                continue
            shuo, *jieqis = line.split(SEP)
            item_lu.append(parse_item(shuo))
            item_so.extend(map(parse_item, jieqis))
        else: # last item line
            info, item = shuo.split(maxsplit=1)
            self.last_dy: bool = hz.dayue_index(info[-1])
        item_lu = del_repeat(item_lu, k=8)
        item_so = del_repeat(item_so, k=8)
        yr: int = self.lead_yr - int(self.lead_jq > 12)
        self.dati_lu: list[dt.Dati] = add_years(item_lu, yr)
        self.dati_so: list[dt.Dati] = add_years(item_so, yr)


    def _init_runs(self: tp.Self) -> None:
        '''
        Picks `run`s among all the lunar `yue`s.

        Assumptions:
            * `dati_lu` starts incl. and ends excl. with `p11`.
            * `dati_so` starts with `dongzhi`, `daxue` or `xiaoxue.

        Adds:
            runs_lu (list[bool]): whether each `yue` is `run`
        '''

        ld_dz: int = 0 if self.lead_jq == 0 else 24 - self.lead_jq
        dati_dz: list[dt.Dati] = self.dati_so[ld_dz::24]
        date_dz: list[dt.Date] = [d.date for d in dati_dz]
        date_lu: list[dt.Date] = [d.date for d in self.dati_lu]
        date_so: list[dt.Date] = [d.date for d in self.dati_so]
        yues_dz: list[int] = [bin_search(date_lu, d) for d in date_dz]
        self.runs_lu: list[bool] = []
        yues_dz.append(len(date_lu))
        for i, (old, new) in enumerate(zip(yues_dz[:-1], yues_dz[1:])):
            cnt: int = new - old
            runs: list[bool] = [False] * cnt
            if cnt > 12:
                dz_old: int = ld_dz + 24 * i
                dz_new: int = dz_old + 24
                de_zq: list[dt.Date] = date_so[dz_old:dz_new:2]
                ys_zq: list[int] = [bin_search(date_lu, d) for d in de_zq]
                rloc: int = min(set(range(cnt)) - {y - old for y in ys_zq})
                runs[rloc] = True
            self.runs_lu.extend(runs)


    def _calc_data_lu(self: tp.Self, csv: bool) -> list[str]:
        '''
        Calculates lunar data (instants of `shuo`) lists.

        Args:
            csv (bool): generates CSV-formatted digit-only data
        Returns:
            list[str]: the well-formatted lunar `shuo` data list
        '''

        datis: list[dt.Dati] = self.dati_lu
        runs: list[bool] = self.runs_lu
        data_lu: list[str] = []
        n, y = self.lead_yr - 1, 11 # nian, yue
        SEP: str = ',' if csv else '.'
        for run, old, new in zip(runs[:-1], datis[:-1], datis[1:]):
            if run:
                n, y = (n - 1, 12) if y <= 1 else (n, y - 1)
            days: int = new.date.days_from(old.date)
            yuefen: hz.Yuefen = hz.Yuefen(run, y, days >= 30)
            info: str = f'{yuefen.ryue:02d}' if csv else yuefen.abbr
            item: str = f'{old.secs:+013d}' if csv else str(old)
            data_lu.append(f'{n:05d}{SEP}{info}{SEP}{item}')
            n, y = (n + 1, 1) if y >= 12 else (n, y + 1)
        else:
            yuefen: hz.Yuefen = hz.Yuefen(runs[-1], y, self.last_dy)
            last: dt.Dati = self.dati_lu[-1]
            info: str = f'{yuefen.ryue:02d}' if csv else yuefen.abbr
            item: str = f'{last.secs:+013d}' if csv else str(last)
            data_lu.append(f'{n:05d}{SEP}{info}{SEP}{item}')
        return data_lu


    def _calc_data_so(self: tp.Self, csv: bool) -> list[str]:
        '''
        Calculates solar data (instants of `jieqi`) lists.

        Args:
            csv (bool): generates CSV-formatted digit-only data
        Returns:
            list[str]: the well-formatted solar `jieqi` data list
        '''

        data_so: list[str] = []
        j: int = self.lead_jq
        s: int = self.lead_yr - int(j > 0)
        SEP: str = ',' if csv else '.'
        for old in self.dati_so:
            info: str = f'{j:02d}' if csv else hz.solar_abbr(j)
            item: str = f'{old.secs:+013d}' if csv else str(old)
            data_so.append(f'{s:05d}{SEP}{info}{SEP}{item}')
            s, j = (s + 1, 0) if j >= 23 else (s, j + 1)
        return data_so


    def __init__(self: tp.Self, in_file: str) -> None:
        with open(in_file, 'r') as ifp_raw:
            self._init_datis(ifp_raw)
        self._init_runs()


    def export(self: tp.Self, out_dir: str, csv: bool) -> None:
        '''
        Calculates and exports lunar and solar data to text files.

        Args:
            out_dir (str): directory the data are exported to
            csv (bool): exports human-read or CSV-formatted data
        '''

        data_lu: list[str] = self._calc_data_lu(csv)
        EXT: str = 'csv' if csv else 'txt'
        out_lu: str = os.path.join(out_dir, f'lunar.{EXT}')
        with open(out_lu, 'w') as ofp_lu:
            if csv:
                ofp_lu.write('cyue,nian,ryue,timestamp\n')
            for i, lu in enumerate(data_lu):
                cyue: str = csv * f'{i:06d},'
                ofp_lu.write(f'{cyue}{lu}\n')
        print(f'lunar data exported to "{out_lu}"')
        data_so: list[str] = self._calc_data_so(csv)
        out_so: str = os.path.join(out_dir, f'solar.{EXT}')
        with open(out_so, 'w') as ofp_so:
            if csv:
                ofp_so.write('cjie,sui,jieqi,timestamp\n')
            for i, so in enumerate(data_so):
                cjie: str = csv * f'{i:06d},'
                ofp_so.write(f'{cjie}{so}\n')
        print(f'solar data exported to "{out_so}"')


def main() -> None:
    here: str = os.path.dirname(__file__)
    build_dir: str = os.path.join(here, 'build')
    raw_path: str = os.path.join(build_dir, 'raw.txt')
    split = Split(in_file=raw_path)
    for csv in (False, True):
        split.export(out_dir=build_dir, csv=csv)


if __name__ == '__main__':
    main()
