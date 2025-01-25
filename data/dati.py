import typing as tp


class Date(tp.NamedTuple):
    '''
    Long-term date with J2G in 1582-10.
    Note: J2G means Julian-to-Gregorian.

    Assumptions:
        * All the entries are in their ranges.

    Attributes:
        y (int): year, integer
        m (int): month, 1..12
        d (int): day, 1..31
    '''

    y: int
    m: int
    d: int


    @property
    def days_since_epoch(self: tp.Self) -> int:
        '''
        Gets days since Unix Epoch (1970-01-01).

        Returns:
            int: days from Unix Epoch incl. to `self` excl.
        '''

        y, m, d = self
        if (m := m - 3) < 0:
            y, m = y - 1, m + 12
        days: int = 30 * m + (3 * m + 2) // 5 + d - 1
        if y >= 1600:
            q, r = divmod(y - 1600, 400)
            days += q * 146097
            q1, r1 = divmod(r, 100)
            days += q1 * 36524
            q2, r2 = divmod(r1, 4)
            days += q2 * 1461 + r2 * 365
        else:
            days += 10 * (self < (1582, 10, 10))
            q, r = divmod(y - 1600, 4)
            days += q * 1461 + r * 365
        # translates from 1600-03-01 to 1970-01-01
        return days - 135080


    def days_from(self: tp.Self, that: 'Date') -> int:
        '''
        Subtracts `that` from `self`.

        Args:
            that (Date): start date of the date interval
        Returns:
            int: days from `that` incl. to `self` excl.
        '''

        days_this: int = self.days_since_epoch
        days_that: int = that.days_since_epoch
        return days_this - days_that


class Time(tp.NamedTuple):
    '''
    24-hour zoneless time within a day.

    Assumptions:
        * All the entries are in their ranges.

    Attributes:
        hh (int): hour, 0..23
        mm (int): minute, 0..59
        ss (int): second, 0..59
    '''

    hh: int
    mm: int
    ss: int


    @property
    def secs_since_midnight(self: tp.Self) -> int:
        '''
        Gets seconds since midnight (00:00:00) of the day.

        Returns:
            int: seconds from midnight incl. to `self` excl.
        '''

        hh, mm, ss = self
        return hh * 3600 + mm * 60 + ss


class Dati(tp.NamedTuple):
    '''
    Long-term dati (date & time UTC+8) with J2G.
    Note: UTC+8 is standard time zone for `nongli`.

    Assumptions:
        * All the entries are in their ranges.

    Attributes:
        y (int): year, integer
        m (int): month, 1..12
        d (int): day, 1..31
        hh (int): hour, 0..23
        mm (int): minute, 0..59
        ss (int): second, 0..59
    '''

    y: int
    m: int
    d: int
    hh: int
    mm: int
    ss: int


    @property
    def date(self: tp.Self) -> Date:
        '''
        Returns:
            Date: date component (y-m-d)
        '''

        return Date(self.y, self.m, self.d)


    @property
    def time(self: tp.Self) -> Time:
        '''
        Returns:
            Time: time component (hh:mm:ss)
        '''

        return Time(self.hh, self.mm, self.ss)


    @property
    def secs(self: tp.Self) -> int:
        '''
        Gets seconds since Unix Epoch (1970-01-01 00:00:00 UTC).

        Returns:
            int: seconds from Unix Epoch incl. to `self` excl.
        '''

        UTC_P8: int = 3600 * 8
        days: int = self.date.days_since_epoch
        secs: int = self.time.secs_since_midnight
        return days * 86400 + secs - UTC_P8


    def secs_from(self: tp.Self, that: 'Dati') -> int:
        '''
        Args:
            that (Dati): start dati of the dati interval
        Returns:
            int: seconds from `that` incl. to `self` excl.
        '''

        return self.secs - that.secs


    def __str__(self: tp.Self) -> str:
        return ('{:05d}' + 5 * '.{:02d}').format(*self)
