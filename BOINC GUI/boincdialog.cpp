#include "boincdialog.h"
#include "ui_boincdialog.h"
#include <QDialog>

Boincdialog::Boincdialog(QWidget *parent) :
    QDialog(parent),
    dialog(new Ui::Boincdialog)
{
dialog->setupUi(this);
}

Boincdialog::~Boincdialog()
{
    delete dialog;
}

void Boincdialog::on_pushButton_clicked()
{
    close();
    exit(0);
}
