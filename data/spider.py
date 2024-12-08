import os
import typing as tp

from selenium import webdriver
from selenium.common.exceptions import NoAlertPresentException
from selenium.webdriver.common.by import By
from selenium.webdriver.remote.webelement import WebElement

def value_input(elem: WebElement, value: int) -> None:
    elem.clear()
    elem.send_keys(f'{value}\n')

def spider_js(url: str, batch: int, out_file: str) -> None:
    '''Do not spider frequently or requests will be blocked'''
    with open(out_file, 'w'):
        pass
    # use your preferred browser
    with webdriver.Firefox() as driver:
        driver.get(url)
        driver.execute_script('showPage(2)')
        ninput: WebElement = driver.find_element(By.ID, 'Cp2_n')
        value_input(elem=ninput, value=500)
        yinput: WebElement = driver.find_element(By.ID, 'Cp2_y')
        for year in range(0, 10000 + 1, batch):
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
    URL_SXWNL: tp.LiteralString = 'https://sxwnl.com/super'
    here: str = os.path.dirname(__file__)
    out_txt: str = os.path.join(here, 'build', 'raw.txt')
    spider_js(url=URL_SXWNL, batch=500, out_file=out_txt)

if __name__ == '__main__':
    main()
