# `Nongli` Changelog

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](CHANGELOG-zh.md)
[![en](https://img.shields.io/badge/lang-en-blue.svg)](CHANGELOG-en.md)


## [Unreleased]

### Added

* Conversions from all `int`s and `float`s by `make_fix64`.

### Changed

* Exception on parsing invalid name in `hanzi.py`.
* `numpy.polynomial.polynomial.polyfit` used on fittings.
* Unsigned integers on possible overflows to avoid UBs.


## [1.1.2] - 2025-06-27 六月初三

### Added

* Functions to check the validity of `date`, `dati`, `riqi`.
* Function `riqi_to_hui` for last day of the `nianyue`.
* Method of test suite to allow custom equality to pass.
* Unit tests for math module.

### Changed

* End-of-line of exported CSV data file into CRLF.
* Fix half-up rounding in function `make_fix64`.
* Fix signedness in function `safe_mul`.

### Removed

* Default copy constructor of `test_suite`.


## [1.1.1] - 2025-05-31 五月初五

### Added

* Appendices of READMEs and vocabulary.
* Declarations on AI-generated documentations and websites.
* New features not introduced by C++17 but helpful for optimizations.
* Function `days_in_cyue` for days contained in a `nongli yue`.
* Unit tests for 4 modules: dati, lunar, solar, `ganzhi`.

### Changed

* Correct ordinal suffixes and plurals the test suite outputs.
* Fix possible out-of-bound access to arrays for `cyue_to_uday`.
* Refactor `coefs.py` to support custom time bounds.
* Replace Miniconda with venv available for free commercial usage.


## [1.0.1] - 2025-05-04 四月初七

### Added

* Function `ganzhi_to_riqi` from `nian`, `yue` and `ganzhi` of day.
* Division to 64-bit fixed-point number between two integers.
* User-defined literals in C++, like `_i64` and `_u64`.
* Page sizes `YD_PAGE` and `JS_PAGE` of multi-page arrays.
* Continuous integration (CI) tests on GitHub Actions.
* Test suite for C++ code in this repository.

### Changed

* Export Jupyter Notebook `coefs.ipynb` to Python script.
* Rename `fix64_t` to `fix64`.
* Correct calculations of total bytes exported.

### Removed

* Check before access to residual arrays.
* Dependency on `ipykernel`.
* APIs for local time and `bazi` from `fix64` longitude.
* Function `math::clip` to clamp within an interval.


## [1.0.0-beta] - 2025-04-14 三月十七

First Pre-Release version.
