'''
Fetches and calculates data and generates raw data text file.
Note: Do not spider too frequently or requests may be blocked.
'''

import os
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


def spider_js(batch: int, out_file: str) -> None:
    '''
    Fetches `nongli` data via JavaScript in the webdriver.

    Args:
        batch (int): how many `sui`s displayed per script
        out_file (str): path to generated raw data text file
    Raises:
        WebDriverException: on bad connection to the site
    '''

    MIN: int = -5000
    MAX: int = 10000
    # use your preferred browser
    with webdriver.Firefox() as driver:
        driver.get('https://sxwnl.com/super')
        driver.execute_script('showPage(2)')
        ninput: WebElement = driver.find_element(By.ID, 'Cp2_n')
        value_input(elem=ninput, value=500)
        yinput: WebElement = driver.find_element(By.ID, 'Cp2_y')
        with open(out_file, 'w'):
            pass
        for year in range(MIN, MAX + 1, batch):
            value_input(elem=yinput, value=year)
            driver.execute_script('getNianLiN()')
            try:
                driver.switch_to.alert.accept()
            except NoAlertPresentException:
                pass
            data: WebElement = driver.find_element(By.ID, 'Cal7')
            with open(out_file, 'a') as raw_file:
                raw_file.write(f'{data.text}\n')
    # time.sleep is unnecessary as only one request is sent


def main() -> None:
    here: str = os.path.dirname(__file__)
    out_txt: str = os.path.join(here, 'build', 'raw.txt')
    spider_js(batch=500, out_file=out_txt)


if __name__ == '__main__':
    main()
