#ifndef SLRWIZARDPAGE_H
#define SLRWIZARDPAGE_H

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWizard>
#include <QWizardPage>

/**
 * @class SLRWizardPage
 * @brief A single step in the SLR(1) guided assistant for table construction.
 *
 * This wizard page presents a specific (state, symbol) cell in the SLR(1) parsing table,
 * and prompts the student to enter the correct ACTION or GOTO value.
 *
 * The page checks the user's input against the expected answer and provides immediate feedback,
 * disabling the "Next" button until the correct response is entered.
 */
class SLRWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a page for a specific cell in the SLR(1) table.
     *
     * @param state The state ID (row index in the table).
     * @param symbol The grammar symbol (column header).
     * @param explanation A pedagogical explanation shown to the user.
     * @param expected The expected answer (e.g., "s2", "r1", "acc", or a state number).
     * @param parent The parent widget.
     */
    SLRWizardPage(int state,
                  const QString &symbol,
                  const QString &explanation,
                  const QString &expected,
                  QWidget *parent = nullptr)
        : QWizardPage(parent)
        , m_state(state)
        , m_symbol(symbol)
        , m_expected(expected)
    {
        setTitle(tr("Estado %1, símbolo '%2'").arg(state).arg(symbol));

        QLabel *lbl = new QLabel(explanation, this);
        lbl->setWordWrap(true);

        m_edit = new QLineEdit(this);
        m_edit->setPlaceholderText(tr("Escribe tu respuesta (p.ej. s3, r2, acc, 5)"));

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(lbl);
        layout->addWidget(m_edit);
        setLayout(layout);

        connect(m_edit, &QLineEdit::textChanged, this, &SLRWizardPage::onTextChanged);
    }
private slots:
    /**
     * @brief Checks the user's input and enables the "Next" button only if correct.
     * @param text The current user input.
     */
    void onTextChanged(const QString &text)
    {
        bool correct = (text.trimmed() == m_expected);
        setComplete(correct);
        if (correct) {
            setSubTitle(tr("✔ Respuesta correcta, pasa a la siguiente pregunta"));
        } else {
            setSubTitle(
                tr("✘ Incorrecto, revisa el enunciado. Consulta los estados que has construido."));
        }
        wizard()->button(QWizard::NextButton)->setEnabled(correct);
    }

private:
    /**
     * @brief Marks the page as complete or incomplete.
     * @param complete Whether the current answer is correct.
     */
    void setComplete(bool complete)
    {
        m_isComplete = complete;
        emit completeChanged();
    }

    /**
     * @brief Returns whether the page is currently complete (required for QWizard).
     * @return true if the correct answer has been entered.
     */
    bool isComplete() const override { return m_isComplete; }

    int m_state;               ///< The state index of the cell.
    QString m_symbol;          ///< The symbol (terminal or non-terminal).
    QString m_expected;        ///< The expected user response.
    QLineEdit *m_edit;         ///< Input field for the user's answer.
    bool m_isComplete = false; ///< Whether the user has entered the correct response.
};

#endif // SLRWIZARDPAGE_H
