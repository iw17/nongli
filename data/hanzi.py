'''
Helps to parse `hanzi` words and output abbreviations.
Note: Please reopen with UTF-8 if it is messed up.
'''

import typing as tp


_LUNAR_HANZI: list[str] = [
    '正月', '二月', '三月', '四月', '五月', '六月',
    '七月', '八月', '九月', '十月', '十一', '十二',
]


def lunar_index(name: str, start: int = 0) -> int:
    '''
    Gets the index of a `yue` by its `hanzi` name.

    Args:
        name (str): name of the `yue` input
        start (int): index of the leading `yue`
    Returns:
        int: biased index of the `yue` input
    Raises:
        ValueError: when name is invalid

    Examples:
        >>> lunar_index('十二', start=0)
        11
        >>> lunar_index('正月', start=1)
        1
        >>> lunar_index('一月', start=1) # invalid name
        ValueError: '一月' is not in list
    '''

    return _LUNAR_HANZI.index(name) + start


_SOLAR_HANZI: list[str] = [
    '冬至', '小寒', '大寒', '立春', '雨水', '惊蛰',
    '春分', '清明', '谷雨', '立夏', '小满', '芒种',
    '夏至', '小暑', '大暑', '立秋', '处暑', '白露',
    '秋分', '寒露', '霜降', '立冬', '小雪', '大雪',
]
_SOLAR_ABBRS: list[str] = [
    'dz', 'xh', 'dh', 'lc', 'ys', 'jz',
    'cf', 'qm', 'gy', 'lx', 'xm', 'mz',
    'xz', 'xs', 'ds', 'lq', 'cs', 'bl',
    'qf', 'hl', 'sj', 'ld', 'xx', 'dx',
]


def solar_index(name: str) -> int:
    '''
    Gets the index of a `jieqi` by its name.

    Args:
        name (str): `hanzi` or abbr of the `jieqi` input
    Returns:
        int: index of the `jieqi` input starting with 0
    Raises:
        ValueError: when name is invalid

    Examples:
    >>> solar_index('立春')
    3
    >>> solar_index('lx') # lowercase only
    9
    >>> solar_index('iw') # invalid name
    ValueError: 'iw' is not in list
    '''

    if name < '\x80':
        return _SOLAR_ABBRS.index(name)
    else:
        return _SOLAR_HANZI.index(name)


def solar_abbr(index: int) -> str:
    '''
    Gets the abbr of a `jieqi` by its index.

    Args:
        index (int): index of the `jieqi` input
        abbr (bool): uses abbr or `hanzi`; default False
    Returns:
        str: abbreviation of the `jieqi` input

    Examples:
    >>> solar_abbr(3)
    'lc'
    '''

    return _SOLAR_ABBRS[index % 24]


_RUN_HANZI: list[str] = ['', '闰']
_RUN_ABBRS: list[str] = ['p', 'r']


def run_abbr(index: bool) -> str:
    '''
    Gets the abbr of the `run`ness of a `yue` input.

    Args:
        index (bool): whether the `yue` input is `run`
    Returns:
        str: abbreviation indicating `run`ness

    Examples:
        >>> run_abbr(True)
        'r'
    '''

    return _RUN_ABBRS[int(index)]


def run_index(name: str) -> bool:
    '''
    Gets whether the input indicates a `runyue`.

    Args:
        name (str): name of the `run`ness input
    Returns:
        bool: whether the input means `run`

    Examples:
        >>> run_index('闰')
        True
        >>> run_index('p')
        False
        >>> run_index('a') # invalid name
        False
    '''

    return name in [_RUN_HANZI[1], _RUN_ABBRS[1]]


_DAYUE_HANZI: list[str] = ['小', '大']
_DAYUE_ABBRS: list[str] = ['x', 'd']


def dayue_abbr(index: bool) -> str:
    '''
    Gets the abbr of the `daxiao` of a `yue` input.

    Args:
        index (bool): whether the `yue` input is `da`
    Returns:
        str: abbreviation indicating `daxiao`

    Examples:
        >>> dayue_name(True)
        'd'
    '''

    return _DAYUE_ABBRS[int(index)]


def dayue_index(name: str) -> bool:
    '''
    Gets whether the input indicates a `dayue`.

    Args:
        name (str): name of the `daxiao` input
    Returns:
        bool: whether the input means `da`
    Raises:
        ValueError: when name is invalid

    Examples:
        >>> dayue_index('x')
        False
        >>> dayue_index('大')
        True
        >>> dayue_index('y') # invalid name
        ValueError: 'y' is not in list
    '''

    if name < '\x80':
        return bool(_DAYUE_ABBRS.index(name))
    else:
        return bool(_DAYUE_HANZI.index(name))


class Yuefen(tp.NamedTuple):
    '''
    Attributes:
        run (bool): whether the `yue` is `run`
        yue (int): index of the `yue` starting with 1
        dax (bool): whether the `yue` is `da`
    '''

    run: bool
    yue: int
    dax: bool


    @staticmethod
    def parse(hanzi: str) -> 'Yuefen':
        '''
        Constructs a `Yuefen` tuple using the `hanzi` input.

        Args:
            hanzi (str): name of the `yuefen` info input
        Returns:
            Yuefen: tuple containing the `yuefen` items
        Raises:
            ValueError: when the input is not well-parsed

        Examples:
        >>> Yuefen.parse('二月大')
        Yuefen(run=False, yue=2, dax=True)
        >>> Yuefen.parse('闰十一小')
        Yuefen(run=True, yue=11, dax=False)
        >>> Yuefen.parse('一二三') # invalid
        ValueError: '一二' is not in list
        '''

        hanzi = hanzi.rstrip()[-4:]
        run: bool = run_index(hanzi[:-3])
        yue: int = lunar_index(hanzi[-3:-1], start=1)
        dax: bool = dayue_index(hanzi[-1:])
        return Yuefen(run, yue, dax)


    @property
    def ryue(self: tp.Self) -> int:
        '''
        Gets the `ryue` value of a `yuefen` info.

        Returns:
            int: `ryue` = `run` + 2 * `yue`

        Examples:
            >>> Yuefen(run=False, yue=1, dax=True).ryue
            2
            >>> Yuefen(run=True, yue=1, dax=True).ryue
            3
        '''

        return (self.yue << 1) | int(self.run)


    @property
    def abbr(self: tp.Self) -> str:
        '''
        Gets the abbr of a `yuefen` info itself.

        Returns:
            str: concatenated abbrs of the 3 attributes

        Examples:
            >>> Yuefen(run=False, yue=2, dax=True).abbr
            'p02d'
            >>> Yuefen(run=True, yue=11, dax=False).abbr
            'r11x'
        '''

        pr: str = run_abbr(self.run)
        xd: str = dayue_abbr(self.dax)
        return f'{pr}{self.yue:02d}{xd}'


    @staticmethod
    def to_abbr(hanzi: str) -> str:
        '''
        Casts the `hanzi` of a `yuefen` info to its abbr.

        Args:
            hanzi (str): name of the `yuefen` info input
        Returns:
            str: concatenated abbrs of the 3 attributes
        Raises:
            ValueError: when the input is not well-parsed

        Examples:
            >>> Yuefen.to_abbr('二月大')
            'p02d'
            >>> Yuefen.to_abbr('闰十一小')
            'r11x'
            >>> Yuefen.to_abbr('一二三') # invalid
            ValueError: '一二' is not in list
        '''

        info: Yuefen = Yuefen.parse(hanzi)
        return info.abbr
