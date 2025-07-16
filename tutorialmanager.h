#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

#include <QEvent>
#include <QFrame>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTextBrowser>
#include <QVector>
#include <QWidget>

/**
 * @struct TutorialStep
 * @brief Represents a single step in the tutorial sequence.
 *
 * Each step highlights a target widget and displays an associated
 * HTML-formatted message.
 */
struct TutorialStep {
    QWidget* target;   ///< Widget to highlight during the tutorial step.
    QString  htmlText; ///< HTML text to show as instruction or explanation.
};

/**
 * @class TutorialManager
 * @brief Manages interactive tutorials by highlighting UI elements and guiding
 * the user.
 *
 * This class implements a step-by-step overlay system that visually highlights
 * widgets and shows textual instructions to guide the user through the
 * interface. It supports multiple tutorials (e.g., for LL(1) and SLR(1) modes),
 * with custom steps and signals for tutorial completion.
 */
class TutorialManager : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Constructs a TutorialManager for a given window.
     * @param rootWindow The main application window used for relative
     * positioning.
     */
    TutorialManager(QWidget* rootWindow);

    /**
     * @brief Adds a new step to the tutorial sequence.
     * @param target The widget to highlight during the step.
     * @param htmlText The instructional HTML message for the step.
     */
    void addStep(QWidget* target, const QString& htmlText);

    /**
     * @brief Starts the tutorial from the beginning.
     */
    void start();

    /**
     * @brief Sets the root window (used for repositioning the overlay).
     * @param newRoot The new main window to reference.
     */
    void setRootWindow(QWidget* newRoot);

    /**
     * @brief Clears all steps in the tutorial.
     */
    void clearSteps();

    /**
     * @brief Hides the tutorial overlay immediately.
     */
    void hideOverlay();

    /**
     * @brief Ends the LL(1) tutorial sequence and emits its corresponding
     * signal.
     */
    void finishLL1();

    /**
     * @brief Ends the SLR(1) tutorial sequence and emits its corresponding
     * signal.
     */
    void finishSLR1();

  protected:
    /**
     * @brief Intercepts UI events to handle overlay behavior.
     */
    bool eventFilter(QObject* obj, QEvent* ev) override;

  signals:
    /**
     * @brief Emitted when a new tutorial step starts.
     * @param index Index of the current step.
     */
    void stepStarted(int index);

    /**
     * @brief Emitted when the full tutorial is finished.
     */
    void tutorialFinished();

    /**
     * @brief Emitted when the LL(1) tutorial ends.
     */
    void ll1Finished();

    /**
     * @brief Emitted when the SLR(1) tutorial ends.
     */
    void slr1Finished();

  public slots:
    /**
     * @brief Advances to the next tutorial step.
     */
    void nextStep();

  private:
    /**
     * @brief Displays the overlay and current step instruction.
     */
    void showOverlay();

    /**
     * @brief Repositions the overlay and highlight rectangle.
     */
    void repositionOverlay();

    QWidget*              m_root;  ///< The main window used for positioning.
    QVector<TutorialStep> m_steps; ///< Ordered list of tutorial steps.
    int                   m_index = -1; ///< Index of the current step.

    QWidget* m_overlay = nullptr; ///< Overlay widget covering the interface.
    QFrame*  m_highlight =
        nullptr; ///< Highlight rectangle around the target widget.
    QTextBrowser* m_textBox = nullptr; ///< Instructional text box.
    QPushButton*  m_nextBtn = nullptr; ///< Button to continue to the next step.
};

#endif // TUTORIALMANAGER_H
