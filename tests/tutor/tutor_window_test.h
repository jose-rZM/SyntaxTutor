#pragma once

#include <QObject>

class TutorWindowTest : public QObject {
    Q_OBJECT

  private slots:
    void createsTutorWithNullTutorialManager();
    void stateAErrorPathAdvancesThroughAStates();
    void stateACorrectPathAdvancesToB();
    void stateBAxiomBranchSkipsB2();
    void stateBDirectAndFallbackPathsUpdateCounters();
    void stateBWrongAnswersStayInB1AndB2();
    void stateCCorrectPathOpensExportActions();
    void stateCWrongAttemptsReachCPrimeAndRecover();
    void tableDialogCancelNoReopensAndYesRequestsExit();
    void tableDialogCancelInCPrimeNoReopensAndYesRequestsExit();
    void exportButtonExportsPdf();
    void exitButtonFinishesWithoutExport();
    void exportsConversationToPdf();
    void smokeGuiFindsCoreWidgets();
    void llAcceptsFlexibleUserFormatting();
    void llAcceptsFlexibleTableCellFormatting();
    void llExamModeWrongAnswersFollowMainPathAndGradeZero();
    void llExamModeAllCorrectScoresTen();
    void llGuidedModeWizardUsesCustomNavigationAndAllowsExit();
    void llGuidedModeWizardCompletesAndReturnsToTable();
    void llGuidedModeAvailableInCPrimeAndHiddenInExam();

    void slrCreatesTutorWithNullTutorialManager();
    void slrStateAErrorPathAdvancesThroughAprime();
    void slrStateACorrectPathAdvancesToB();
    void slrStateBAndCIncorrectAnswersStillAdvance();
    void slrStateCAWrongThenCorrect();
    void slrStateCBEpsilonBranchAcceptsOnlyEmpty();
    void slrStateCBNonEpsilonBranchAdvancesOnWrongAndCorrect();
    void slrDriveThroughCollectionUntilD();
    void slrStateDErrorPathAdvancesToE();
    void slrStateEErrorPathAdvancesToF();
    void slrStateFNoConflictAdvancesToG();
    void slrStateFConflictBranchAdvancesToFAAndThenG();
    void slrLr0ConflictsIgnoreTheAcceptItem();
    void slrStateGWrongThenCorrectReachesH();
    void slrStateHIncorrectTableKeepsDialogOpen();
    void slrGuidedModeWizardUsesCustomNavigationAndAllowsExit();
    void slrGuidedModeWizardCompletesAndReturnsToTable();
    void slrTableDialogCancelNoReopensAndYesRequestsExit();
    void slrFinalTableCorrectPathExportsAndExits();
    void slrSmokeGuiFindsCoreWidgets();
    void slrAcceptsFlexibleUserFormatting();
    void slrAcceptsFlexibleTableCellFormatting();
    void slrExamModeWrongAnswersFollowMainPathAndShowReport();
    void slrAutomatonHiddenInExamMode();
    void slrAutomatonRevealsI0AfterInitialConstruction();
    void slrAutomatonProgressivelyRevealsAndCompletesAfterLoop();
    void slrAutomatonHighlightsConflictAndReduceStates();
    void slrAutomatonButtonGatingAndViewerReuse();
    void slrAutomatonViewerUpdatesLiveAndReopens();

    void mainInitialUiIsVisibleAndEnabled();
    void mainSwitchLanguageToEnglishPersistsSelection();
    void mainSwitchLanguageToSpanishPersistsSelection();
    void mainAboutDialogShowsMetadata();
    void mainQuickReferencesOpen();
    void mainLlAndSlrEntryPointsOpenTutors();
    void mainTutorialFlowCompletesAndReenablesControls();
    void mainGamificationPersistsAcrossRestart();
    void mainStatePersistenceAcrossRestart();
    void typographyStyleSheetDeclaresSizesOnlyThroughRoles();
    void typographyResolvesRolesAtTheCurrentTextScale();
    void typographyAppliesPointSizesToWidgets();
    void typographyLengthsForTextFollowTheRenderedBodyText();
    void typographyAnswerBoxGrowsByLinesOfItsOwnFont();

    void layoutHomeShowsAllTextAtEveryTextSize();
    void layoutDialogsShowAllTextAtEveryTextSize();
    void layoutTutorsShowAllTextAtEveryTextSize();
    void layoutScrollBarsFollowTheDarkTheme();

    void mainCustomGrammarToggleDisablesLevels();
    void mainCustomGrammarLlFlowStartsTutorWithUserGrammar();
    void mainCustomGrammarSlrFlowStartsTutorWithUserGrammar();
    void mainCustomGrammarEditorRejectsInvalidAndNonLl1Grammars();
    void mainCustomGrammarEditorCancelKeepsHomePage();
    void mainCustomGrammarEditorRestoresLastGrammar();
    void llUserGrammarKeepsWrittenRuleOrder();
    void slrUserGrammarKeepsWrittenRuleOrder();
    void mainExamModeCheckboxLaunchesExamTutor();
};
