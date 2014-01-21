#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>

namespace Ui {
class Boincdialog;
}
class Boincdialog : public QDialog{
    Q_OBJECT

public:
    explicit Boincdialog( QWidget * parent = 0);
    ~Boincdialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Boincdialog *dialog;
};
#endif // DIALOG_H
