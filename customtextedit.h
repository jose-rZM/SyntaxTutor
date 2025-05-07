#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>

class CustomTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CustomTextEdit(QWidget *parent = nullptr);

signals:
    void sendRequested();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // CUSTOMTEXTEDIT_H
