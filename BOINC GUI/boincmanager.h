#ifndef BOINCMANAGER_H
#define BOINCMANAGER_H

#include <QMainWindow>
#include <QDialog>

namespace Ui {
class Boincmanager;
}


class Boincmanager : public QMainWindow
{
    Q_OBJECT

public:
    explicit Boincmanager(QWidget *parent = 0);
    ~Boincmanager();

private slots:
    void on_start_clicked();
    void on_stop_clicked();
    void on_tabWidget_currentChanged(int index);

    void on_createac_2_clicked();

    void on_attach_clicked();

    void on_detach_clicked();

    void on_reset_clicked();

    void on_update_clicked();

    void on_resume_clicked();

    void on_suspend_clicked();

    void on_tabWidget_2_currentChanged(int index);

private:
    Ui::Boincmanager *ui;
};

#endif // BOINCMANAGER_H
