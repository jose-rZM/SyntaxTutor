#ifndef SLRWIZARDPAGE_H
#define SLRWIZARDPAGE_H

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWizard>
#include <QWizardPage>

class SLRWizardPage : public QWizardPage
{
    Q_OBJECT
public:
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
        setTitle(QString("Estado %1, símbolo '%2'").arg(state).arg(symbol));

        QLabel *lbl = new QLabel(explanation, this);
        lbl->setWordWrap(true);

        m_edit = new QLineEdit(this);
        m_edit->setPlaceholderText("Escribe tu respuesta (p.ej. s3, r2, acc, 5)");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(lbl);
        layout->addWidget(m_edit);
        setLayout(layout);

        connect(m_edit, &QLineEdit::textChanged, this, &SLRWizardPage::onTextChanged);
    }
private slots:
    void onTextChanged(const QString &text)
    {
        bool correct = (text.trimmed() == m_expected);
        setComplete(correct);
        if (correct) {
            setSubTitle("✔ Respuesta correcta, pasa a la siguiente pregunta");
        } else {
            setSubTitle(
                "✘ Incorrecto, revisa el enunciado. Consulta los estados que has construido.");
        }
        wizard()->button(QWizard::NextButton)->setEnabled(correct);
    }

private:
    void setComplete(bool complete)
    {
        m_isComplete = complete;
        emit completeChanged();
    }
    bool isComplete() const override { return m_isComplete; }

    int m_state;
    QString m_symbol;
    QString m_expected;
    QLineEdit *m_edit;
    bool m_isComplete = false;
};

#endif // SLRWIZARDPAGE_H
