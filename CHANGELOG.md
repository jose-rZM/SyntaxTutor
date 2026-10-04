# Changelog

All notable changes to this project will be documented in this file.

## [2.0.0] - 2026-10-04
A major release that turns SyntaxTutor into a standalone project: new features for practice and
assessment, a redesigned interface and a new visual identity. Settings from 1.x are not migrated.

### Added
- Custom grammars: an editor with live validation that only accepts LL(1) or SLR(1) grammars,
  supports multi-character symbols and remembers the last grammar.
- Exam mode: no feedback or sub-questions during the exercise, a 0-10 grade with per-cell credit
  for the final table, and a question-by-question report that can be exported to PDF.
- LR(0) automaton viewer, drawn progressively as the student builds states and transitions.
- Guided mode for the LL(1) table, to fill it in one cell at a time.
- Adjustable text size, and LL(1)/SLR(1) quick references in the About menu.
- Native Wayland support on Linux (X11 through XWayland stays the default).
- New app icon and logo, used in the app, the PDFs and the manuals.
- Updated user manuals in English and Spanish.
- GUI test suite and a CI workflow that runs the backend and GUI tests.

### Changed
- Redesigned interface: single window with stacked pages, a new dark theme and typography in
  points, so text keeps its size across Windows, macOS and Linux.
- SLR guided mode is a custom dialog, consistent on every platform.
- Redesigned PDF exports for LL(1), SLR(1) and exam reports, with sorted sets and states, the
  same table notation as the exercise, and wide tables split into column blocks.
- User grammars are shown and numbered in the order they were written.
- Settings moved to the new "SyntaxTutor" organisation (macOS bundle id `me.jram.syntaxtutor`).
- Releases: macOS builds for Apple Silicon only; the Linux AppImage targets Ubuntu 22.04 or newer.

### Fixed
- SLR(1) conflicts between accept and reduce on `$` were missed depending on the platform.
- An accepting item next to a reduction is now treated as an LR(0) conflict in question F.
- Questions and feedback about the initial item showed `S -> A $` instead of the grammar's own.
- Exporting an SLR(1) PDF could read past the end of the transitions of a state.
- PDF export reports failures instead of failing silently.
- Text sizes on high-DPI Windows and Linux screens, low-contrast text and white-on-white text with
  light system palettes.
- Chat bubbles were too narrow and wrapped the last word of a line, such as `A ->` / `B`.
- Several English translations, including the tutorial's next button.

## [1.0.4] - 2025-11-08
Minor usability improvements in user input handling and enhancements to the pre-release workflow to simlify future releases.
### Added
- Tutorial hint explaining the expected format used in SLR table question (H)

### Fixed
- Remove all spaces in LL/SLR table questions (C-C' and H steps)

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
