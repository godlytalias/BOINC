#include "boincmanager.h"
#include "ui_boincmanager.h"
#include <pthread.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringmatcher.h>
#include <QTime>

pthread_t tid1,tid2;
pthread_attr_t attr1,attr2;
bool started=false;
QString line="";
QTime dieTime;
Boincmanager::Boincmanager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Boincmanager)
{
    ui->setupUi(this);
}

Boincmanager::~Boincmanager()
{
    system("./BOINC/boinccmd --quit");
    system("rm started.txt");
    delete ui;
}

void *runner(void* param)
{
    system("./BOINC/boinc > started.txt");
}

void *updater(void* param)
{
    QFile file("started.txt");
    file.open (QIODevice::ReadOnly);
    line="";
    QTextStream stream ( &file );
    while( !stream.atEnd()) {
        line += stream.readLine()+"\n";
    }
    file.close();
}

void delay(int sec)
{
    dieTime= QTime::currentTime().addSecs(sec);
        while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void Boincmanager::on_start_clicked()
{
    started=true;
    pthread_attr_init (&attr1);
    pthread_create(&tid1,&attr1,runner,NULL);
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);
    ui->textEdit->setText("BOINC started\nLoading...");
    delay(2);
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,updater,NULL);
    delay(4);
       ui->textEdit->setText(line);
}


void Boincmanager::on_stop_clicked()
{
    started=false;
    system("./BOINC/boinccmd --quit");
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);
    delay(2);
    QFile file("started.txt");
    file.open (QIODevice::ReadOnly);
    QTextStream stream ( &file );
    line="";
    while( !stream.atEnd()) {
        line += stream.readLine()+"\n";
    }
    file.close();
    ui->textEdit->setText(line);
}



void Boincmanager::on_tabWidget_currentChanged(int index)
{
    ui->projectext->setText("");
    if(index==2){
     ui->progressBar->setValue(90);}
}

void Boincmanager::on_createac_2_clicked()
{
    if(started){
    QString cmd1 = "./BOINC/boinccmd --create_account ";
    QString cmd2 = ui->prurl1->text()+" ";
    QString cmd3 = ui->email->text()+" ";
    QString cmd4 = ui->pwd->text()+" ";
    QString cmd5 = ui->uname->text();
    QString cmd6 = " > created.txt";
    QString cmd = cmd1+cmd2+cmd3+cmd4+cmd5+cmd6;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Creating account...");
    delay(3);
    QFile file("created.txt");
    file.open (QIODevice::ReadOnly);
    QTextStream stream ( &file );
    line="";
    while( !stream.atEnd()) {
        line += stream.readLine()+"\n";
    }
    file.close();
    ui->projectext->setText(line);
    system("rm created.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_attach_clicked()
{
    if(started){
    QString cmd1 = "./BOINC/boinccmd --project_attach ";
    QString cmd2 = ui->prurl2->text()+" ";
    QString cmd3 = ui->auth->text();
    QString cmd = cmd1+cmd2+cmd3;
     system(cmd.toUtf8().constData());
     ui->projectext->setText("Attaching...");
     delay(2);
     QFile file("attached.txt");
    file.open (QIODevice::ReadOnly);
    QTextStream stream ( &file );
    line="";
    while( !stream.atEnd()) {
        line += stream.readLine()+"\n";
    }
    file.close();
    ui->projectext->setText(line);
    system("rm attached.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_detach_clicked()
{
    if(started){
   QString cmd1 = "./BOINC/boinccmd --project ";
   QString cmd2 = ui->prurl2->text();
   QString cmd3 = " detach > detached.txt";
   QString cmd = cmd1+cmd2+cmd3;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Detaching project...");
    delay(2);
        QFile file("detached.txt");
        file.open (QIODevice::ReadOnly);
        QTextStream stream ( &file );
        line="";
        while( !stream.atEnd()) {
            line += stream.readLine()+"\n";
        }
        file.close();
        ui->projectext->setText(line);
        system("rm detached.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_reset_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" reset > reset.txt";
    QString cmd=cmd1+cmd2+cmd3;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Resetting project...");
    delay(2);
        QFile file("reset.txt");
        file.open (QIODevice::ReadOnly);
        QTextStream stream ( &file );
        line="";
        while( !stream.atEnd()) {
            line += stream.readLine()+"\n";
        }
        file.close();
        ui->projectext->setText(line);
        system("rm reset.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_update_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" update > update.txt";
    QString cmd=cmd1+cmd2+cmd3;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Updating project...");
    delay(2);
        QFile file("update.txt");
        file.open (QIODevice::ReadOnly);
        QTextStream stream ( &file );
        line="";
        while( !stream.atEnd()) {
            line += stream.readLine()+"\n";
        }
        file.close();
        ui->projectext->setText(line);
        system("rm update.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_resume_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" resume > resume.txt";
    QString cmd=cmd1+cmd2+cmd3;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Resuming project...");
    delay(2);
        QFile file("resume.txt");
        file.open (QIODevice::ReadOnly);
        QTextStream stream ( &file );
        line="";
        while( !stream.atEnd()) {
            line += stream.readLine()+"\n";
        }
        file.close();
        ui->projectext->setText(line);
        system("rm resume.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_suspend_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" suspend > suspend.txt";
    QString cmd=cmd1+cmd2+cmd3;
    system(cmd.toUtf8().constData());
    ui->projectext->setText("Suspending project...");
    delay(2);
        QFile file("suspend.txt");
        file.open (QIODevice::ReadOnly);
        QTextStream stream ( &file );
        line="";
        while( !stream.atEnd()) {
            line += stream.readLine()+"\n";
        }
        file.close();
        ui->projectext->setText(line);
        system("rm suspend.txt");
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_tabWidget_2_currentChanged(int index)
{
    ui->prurl1->setText("");
    ui->prurl2->setText("");
    ui->prurl3->setText("");
    ui->auth->setText("");
    ui->email->setText("");
    ui->uname->setText("");
    ui->pwd->setText("");

}
