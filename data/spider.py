'''
Fetches and calculates data and generates raw data text file.
Note: Do not spider too frequently or requests may be blocked.
'''

import argparse as ap
import os
import time
import typing as tp

from selenium import webdriver
from selenium.common.exceptions import NoAlertPresentException
from selenium.webdriver.common.by import By
from selenium.webdriver.remote.webelement import WebElement


def value_input(elem: WebElement, value: int) -> None:
    '''
    Inputs an integer value into the webdriver element.

    Args:
        elem (WebElement): in which the input value is filled
        value (int): number input into the webdriver element
    '''

    elem.clear()
    elem.send_keys(f'{value}\n')


class Config(tp.NamedTuple):
    '''
    Configurations for time bounds to export.

    Assumptions:
        * `upper > lower`.

    Attributes:
        lower (int): minimal `nian`, `sui` and year
        upper (int): maximal `nian`, `sui` and year
    '''

    lower: int
    upper: int


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
    parser.add_argument('-l', '--lower', **kw_min)
    parser.add_argument('-u', '--upper', **kw_max)
    args: ap.Namespace = parser.parse_args()
    return Config(args.lower, args.upper)


def spider_js(batch: int, out_file: str, conf: Config) -> None:
    '''
    Fetches `nongli` data via JavaScript in the webdriver.

    Args:
        batch (int): how many `sui`s displayed per script
        out_file (str): path to generated raw data text file
    Raises:
        WebDriverException: on bad connection to the site
    '''

    lo, hi = conf
    # use your preferred browser
    with webdriver.Firefox() as driver:
        driver.get('https://sxwnl.com/super')
        driver.execute_script('showPage(2)')
        ninput: WebElement = driver.find_element(By.ID, 'Cp2_n')
        value_input(elem=ninput, value=500)
        yinput: WebElement = driver.find_element(By.ID, 'Cp2_y')
        with open(out_file, 'w'):
            pass
        for year in range(lo, hi + 1, batch):
            value_input(elem=yinput, value=year)
            driver.execute_script('getNianLiN()')
            try:
                driver.switch_to.alert.accept()
            except NoAlertPresentException:
                pass
            data: WebElement = driver.find_element(By.ID, 'Cal7')
            with open(out_file, 'a') as raw_file:
                raw_file.write(f'{data.text}\n')
            time.sleep(5) # secs per batch


def main() -> None:
    here: str = os.path.dirname(__file__)
    out_dir: str = os.path.join(here, 'build')
    os.makedirs(out_dir, exist_ok=True)
    out_txt: str = os.path.join(out_dir, 'raw.txt')
    conf: Config = load_config()
    spider_js(batch=500, out_file=out_txt, conf=conf)


if __name__ == '__main__':
    main()
