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
    void slrStateGWrongThenCorrectReachesH();
    void slrStateHIncorrectTableKeepsDialogOpen();
    void slrGuidedModeWizardCompletesAndReturnsToTable();
    void slrTableDialogCancelNoReopensAndYesRequestsExit();
    void slrFinalTableCorrectPathExportsAndExits();
    void slrSmokeGuiFindsCoreWidgets();

    void mainInitialUiIsVisibleAndEnabled();
    void mainSwitchLanguageToEnglishPersistsSelection();
    void mainSwitchLanguageToSpanishPersistsSelection();
    void mainAboutDialogShowsMetadata();
    void mainQuickReferencesOpen();
    void mainLlAndSlrEntryPointsOpenTutors();
    void mainTutorialFlowCompletesAndReenablesControls();
    void mainGamificationPersistsAcrossRestart();
    void mainStatePersistenceAcrossRestart();
};
