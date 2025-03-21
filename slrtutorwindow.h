#ifndef SLRTUTORWINDOW_H
#define SLRTUTORWINDOW_H

#include <QMainWindow>

namespace Ui {
class slrtutorwindow;
}

class slrtutorwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit slrtutorwindow(QWidget *parent = nullptr);
    ~slrtutorwindow();

private:
    Ui::slrtutorwindow *ui;
};

#endif // SLRTUTORWINDOW_H
