# Changelog

All notable changes to this project will be documented in this file.

## [1.0.3] - 2025-11-04
Minor usability improvements in user input handling.
### Added
- Allow the user to write `A -> .` instead of `A -> EPSILON .`

### Fixed
- Accept input containing spaces in responses sush as `x, y`

## [1.0.2] - 2025-07-16
### Added
- User manual in Spanish (`manual/SyntaxTutor-Manual-ES.pdf`)
- User manual in English (`manual/SyntaxTutor-Manual-EN.pdf`)
- Developer manual (`manual/SyntaxTutor-Developer-Manual.pdf`)
- Script to customize titlepage (`manual/patch_refman_title.sh`)

### Fixed
- Fixed issue when exporting PDF in SLR mode.
- Fixed some feedback in SLR mode

## [1.0.1] - 2025-06-17
### Added
- Added `Doxyfile` for automatic documentation generation with Doxygen.
- Completed missing translations for multilingual support (English/Spanish).

### Fixed
- Corrected a typo in the SLR(1) Quick Reference view.
- EPSILON is no longer shown when exporting LL(1) parse tables to PDF.
- Improved feedback message for the FA question in the SLR module.

### Quality
- All changes successfully passed CI (GitHub Actions).
- Test suite: 158 tests passed (100% success rate).
- Maintained high test coverage across modules (most above 90%).

## [1.0.0] - 2025-06-15
### Initial Release
- First public version of SyntaxTutor.
- Includes LL(1) and SLR(1) modules with guided exercises.
- Features interactive tutoring, automatic grammar generation, feedback system, and performance tracking.
