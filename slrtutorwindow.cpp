#include "slrtutorwindow.h"
#include "ui_slrtutorwindow.h"

slrtutorwindow::slrtutorwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::slrtutorwindow)
{
    ui->setupUi(this);
}

slrtutorwindow::~slrtutorwindow()
{
    delete ui;
}
