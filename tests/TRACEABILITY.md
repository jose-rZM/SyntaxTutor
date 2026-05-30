# Traceability Matrix

This document maps each manual functional test case under `qa/test-cases/` to the automated Qt Test coverage in `tests/tutor/`.

## Fixture Groups

### LL(1)

- `llFixtures()`
  - `ll-simple`
  - `ll-epsilon`
  - `ll-branching`

### SLR(1)

- `slrNoConflictFixtures()`
  - `slr-simple`
  - `slr-chain`
- `slrConflictFixtures()`
  - `slr-conflict`
- `slrFixturesWithCbEpsilon()`
- `slrFixturesWithCbNonEpsilon()`

## Main App

| Manual Case | Automated Test | File | Fixture Scope | Notes |
|---|---|---|---|---|
| `MAIN-TC-01` | `mainInitialUiIsVisibleAndEnabled` | `main_window_test.cpp` | N/A | Initial UI visibility and enabled controls |
| `MAIN-TC-02` | `mainSwitchLanguageToEnglishPersistsSelection` | `main_window_test.cpp` | N/A | Uses test settings and avoids real restart under `SYNTAXTUTOR_TESTING` |
| `MAIN-TC-03` | `mainSwitchLanguageToSpanishPersistsSelection` | `main_window_test.cpp` | N/A | Uses test settings and avoids real restart under `SYNTAXTUTOR_TESTING` |
| `MAIN-TC-04` | `mainAboutDialogShowsMetadata` | `main_window_test.cpp` | N/A | About dialog smoke/content check |
| `MAIN-TC-05` | `mainQuickReferencesOpen` | `main_window_test.cpp` | N/A | LL and SLR quick references |
| `MAIN-TC-06` | `mainTutorialFlowCompletesAndReenablesControls` | `main_window_test.cpp` | N/A | Full tutorial smoke path |
| `MAIN-TC-07` | `llAcceptsFlexibleUserFormatting` | `ll_tutor_window_test.cpp` | `llFixtures()` | Covered in tutor suite rather than main smoke suite |
| `MAIN-TC-08` | `tableDialogCancelNoReopensAndYesRequestsExit`, `tableDialogCancelInCPrimeNoReopensAndYesRequestsExit` | `ll_tutor_window_test.cpp` | specific LL fixture | Covered in tutor suite |
| `MAIN-TC-09` | `mainLlAndSlrEntryPointsOpenTutors`, `slrStateHIncorrectTableKeepsDialogOpen` | `main_window_test.cpp`, `slr_tutor_window_test.cpp` | mixed | Main covers entry point; tutor suite covers table processing |
| `MAIN-TC-10` | `exportButtonExportsPdf`, `slrFinalTableCorrectPathExportsAndExits` | `ll_tutor_window_test.cpp`, `slr_tutor_window_test.cpp` | LL/SLR fixture groups | Export flow covered in tutor suites |
| `MAIN-TC-11` | `mainGamificationPersistsAcrossRestart` | `main_window_test.cpp` | N/A | Simulates finished tutor session |
| `MAIN-TC-12` | `mainStatePersistenceAcrossRestart` | `main_window_test.cpp` | N/A | Reads persisted test settings |

## LL(1)

| Manual Case | Automated Test | File | Fixture Scope | Notes |
|---|---|---|---|---|
| `LL1-TC-01` | `createsTutorWithNullTutorialManager` | `ll_tutor_window_test.cpp` | `llFixtures()` | Session creation and initial state |
| `LL1-TC-02` | `stateAErrorPathAdvancesThroughAStates` | `ll_tutor_window_test.cpp` | `llFixtures()` | Full A -> A1 -> A2 -> A' path |
| `LL1-TC-03` | `stateACorrectPathAdvancesToB` | `ll_tutor_window_test.cpp` | `llFixtures()` | Direct A -> B path |
| `LL1-TC-04` | `stateBDirectAndFallbackPathsUpdateCounters`, `stateBWrongAnswersStayInB1AndB2` | `ll_tutor_window_test.cpp` | specific LL fixture | Non-axiom B branch coverage |
| `LL1-TC-05` | `stateBAxiomBranchSkipsB2` | `ll_tutor_window_test.cpp` | specific LL fixture | Axiom branch skips B2 |
| `LL1-TC-06` | `stateBDirectAndFallbackPathsUpdateCounters` | `ll_tutor_window_test.cpp` | specific LL fixture | Direct correct B answer |
| `LL1-TC-07` | `stateCCorrectPathOpensExportActions`, `exportButtonExportsPdf`, `exitButtonFinishesWithoutExport` | `ll_tutor_window_test.cpp` | mixed | Final state actions and export |
| `LL1-TC-08` | `stateCWrongAttemptsReachCPrimeAndRecover` | `ll_tutor_window_test.cpp` | specific LL fixture | Retry limit and C' recovery |
| `LL1-TC-09` | `tableDialogCancelNoReopensAndYesRequestsExit`, `tableDialogCancelInCPrimeNoReopensAndYesRequestsExit` | `ll_tutor_window_test.cpp` | specific LL fixture | Cancel flow in C and C' |
| `Interno` | `exportsConversationToPdf` | `ll_tutor_window_test.cpp` | `ll-epsilon` | Direct export helper validation |
| `Interno` | `smokeGuiFindsCoreWidgets` | `ll_tutor_window_test.cpp` | `llFixtures()` | Minimal GUI smoke |
| `Interno` | `llAcceptsFlexibleUserFormatting` | `ll_tutor_window_test.cpp` | `llFixtures()` | Input formatting tolerance |
| `Interno` | `llAcceptsFlexibleTableCellFormatting` | `ll_tutor_window_test.cpp` | `llFixtures()` | Table cell spacing tolerance |

## SLR(1)

| Manual Case | Automated Test | File | Fixture Scope | Notes |
|---|---|---|---|---|
| `SLR1-TC-01` | `slrCreatesTutorWithNullTutorialManager` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Session creation and initial state |
| `SLR1-TC-02` | `slrStateACorrectPathAdvancesToB` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Direct A -> B path |
| `SLR1-TC-03` | `slrStateAErrorPathAdvancesThroughAprime` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Full A error branch |
| `SLR1-TC-04` | `slrStateBAndCIncorrectAnswersStillAdvance` | `slr_tutor_window_test.cpp` | specific SLR fixture | Matches current tutor behavior |
| `SLR1-TC-05` | `slrStateCAWrongThenCorrect` | `slr_tutor_window_test.cpp` | specific SLR fixture | CA retry behavior |
| `SLR1-TC-06` | `slrStateCBEpsilonBranchAcceptsOnlyEmpty` | `slr_tutor_window_test.cpp` | `slrFixturesWithCbEpsilon()` | CB empty branch |
| `SLR1-TC-07` | `slrStateCBNonEpsilonBranchAdvancesOnWrongAndCorrect` | `slr_tutor_window_test.cpp` | `slrFixturesWithCbNonEpsilon()` | CB non-empty branch |
| `SLR1-TC-08` | `slrDriveThroughCollectionUntilD` | `slr_tutor_window_test.cpp` | specific SLR fixture | Collection loop until D |
| `SLR1-TC-10` | `slrStateDErrorPathAdvancesToE` | `slr_tutor_window_test.cpp` | specific SLR fixture | D error path |
| `SLR1-TC-12` | `slrStateEErrorPathAdvancesToF` | `slr_tutor_window_test.cpp` | specific SLR fixture | E error path |
| `SLR1-TC-13`, `SLR1-TC-14`, `SLR1-TC-15` | `slrStateFConflictBranchAdvancesToFAAndThenG` | `slr_tutor_window_test.cpp` | `slrConflictFixtures()` | Conflict detection and FA resolution |
| `SLR1-TC-16` | `slrStateFNoConflictAdvancesToG` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | No-conflict F branch |
| `SLR1-TC-17` | `slrStateGWrongThenCorrectReachesH` | `slr_tutor_window_test.cpp` | specific SLR fixture | G retry and completion |
| `SLR1-TC-18` | `slrStateHIncorrectTableKeepsDialogOpen`, `slrGuidedModeWizardCompletesAndReturnsToTable` | `slr_tutor_window_test.cpp` | specific SLR fixture | Adapted to current UI: guided mode via button |
| `SLR1-TC-19` | `slrFinalTableCorrectPathExportsAndExits` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Final table, PDF export, exit |
| `SLR1-TC-20` | `slrTableDialogCancelNoReopensAndYesRequestsExit` | `slr_tutor_window_test.cpp` | specific SLR fixture | Table dialog cancel flow |
| `Interno` | `slrSmokeGuiFindsCoreWidgets` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Minimal GUI smoke |
| `Interno` | `slrAcceptsFlexibleUserFormatting` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()`, `slrConflictFixtures()` | Input formatting tolerance |
| `Interno` | `slrAcceptsFlexibleTableCellFormatting` | `slr_tutor_window_test.cpp` | `slrNoConflictFixtures()` | Table cell spacing tolerance |
