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
    system("./BOINC/boinccmd -V > version.txt");
    QFile file("version.txt");
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    line=stream.readLine();
    QStringList ls;
    ls=line.split(" ", QString::SkipEmptyParts);
    line="Version ";
    line+=ls.at(4);
    ui->label_10->setText(line);
    file.close();
    system("rm version.txt");
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
    ui->projectext->setEnabled(true);
    ui->textEdit->setText("BOINC started\nLoading...");
    delay(2);
    ui->tab_2->setEnabled(true);
    ui->tabWidget_2->setEnabled(true);
    ui->createac->setEnabled(true);
    ui->tab_3->setEnabled(true);
    ui->tab_4->setEnabled(true);
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,updater,NULL);
    delay(2);
       ui->textEdit->setText(line);
}


void Boincmanager::on_stop_clicked()
{
    started=false;
    system("./BOINC/boinccmd --quit");
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);
    ui->tab_2->setDisabled(true);
    ui->tab_3->setDisabled(true);
    ui->tab_4->setDisabled(true);
    ui->tabWidget_2->setDisabled(true);
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
    if(index==0){
        pthread_attr_init(&attr2);
        pthread_create(&tid2,&attr2,updater,NULL);
        delay(1);
           ui->textEdit->setText(line);
    }
    if(index==3){
        system("./BOINC/boinccmd --get_notices 0 > notices.txt");
        QFile file("notices.txt");
        line="";
        file.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        if(stream.atEnd())
            ui->notices->setText(" NO NEW NOTICES ");
        else
        {
            while(!stream.atEnd())
            {
                line+=stream.readLine();
                line+="\n";
            }
            ui->notices->setText(line);
        }
        file.close();
        system("rm notices.txt");
    }
    if((index==2) && started){
        system("./BOINC/boinccmd --get_disk_usage > diskusage.txt");
        QFile file("diskusage.txt");
        file.open(QIODevice::ReadOnly);
        QTextStream stream (&file);
        line="";
        QString temp;
        QStringList list;
        double total=1.0,free=1.0;
        double value;
        while(!stream.atEnd()){
            temp=stream.readLine();
            if(temp.startsWith("total"))
            {
                list=temp.split(": ",QString::SkipEmptyParts);
                total=list.at(1).toDouble();
            }
            if(temp.startsWith("free"))
            {
                list=temp.split(": ",QString::SkipEmptyParts);
                free=list.at(1).toDouble();
            }
        }
        file.close();
        value=((total-free)/total)*100;
        system("rm diskusage.txt");
        ui->progressBar->setValue((int)value);

        system("./BOINC/boinccmd --get_project_status > projectstatus.txt");
        QFile status("projectstatus.txt");
        status.open(QIODevice::ReadOnly);
        QTextStream statstream(&status);
        line="";
        int i=1;
        QString no;
        list.clear();
        while(!statstream.atEnd())
        {
            no=QString::number(i,10);
            if(statstream.readLine().startsWith(no))
            {
                if(i>1)
                    line+="\n______________________________________________\n\n";
                list=statstream.readLine().split(": ");
                line+="PROJECT NAME\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\nPROJECT URL\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\nUSER NAME\t\t: ";
                line+=list.at(1);
                statstream.readLine();statstream.readLine();
                list=statstream.readLine().split(": ");
                line+="\nUSER TOTAL CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\nUSER AVG. CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\nHOST TOTAL CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\nHOST AVG. CREDIT\t\t: ";
                line+=list.at(1);
                i++;
            }
        }
        system("rm projectstatus.txt");
        ui->textEdit_2->setText(line);
        system("./BOINC/boinccmd --get_host_info > hostinfo.txt");
        QFile fil("hostinfo.txt");
        fil.open(QIODevice::ReadOnly);
        QTextStream tstream(&fil);
        list.clear();
        tstream.readLine();//skipping 1st line
        temp=tstream.readLine();
        list=temp.split(": ");
        ui->domname->setText(list.at(1));
        tstream.readLine();tstream.readLine();tstream.readLine(); //skipping 3 lines
        temp=tstream.readLine();
        list=temp.split(": ");
        ui->cpu->setText(list.at(1));
        list=tstream.readLine().split(": ");
        ui->fp->setText(list.at(1));
        list=tstream.readLine().split(": ");
        ui->int_2->setText(list.at(1));
        tstream.readLine();
        list=tstream.readLine().split(": ");
        temp=list.at(1);
        list=tstream.readLine().split(": ");
        temp+=" ";
        temp+=list.at(1);
        ui->os->setText(temp);
        list=tstream.readLine().split(": ");
        ui->memory->setText(list.at(1));
        temp=tstream.readLine();
        list=temp.split(": ");
        ui->cache->setText(list.at(1));
        fil.close();
        system("rm hostinfo.txt");
    }
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
    QString cmd4 = " > attached.txt";
    QString cmd = cmd1+cmd2+cmd3+cmd4;
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
