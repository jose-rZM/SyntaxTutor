# Changelog

All notable changes to this project will be documented in this file.

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
