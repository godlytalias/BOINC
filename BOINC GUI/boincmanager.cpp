#include "boincmanager.h"
#include "boincdialog.h"
#include "ui_boincmanager.h"
#include <pthread.h>
#include <qdialog.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringmatcher.h>
#include <QTime>
#include <qdir.h>
#include <qfile.h>

pthread_t tid1,tid2;
pthread_attr_t attr1,attr2;
bool started=false,done=false;
QString line="",cmd;
QString temp;
int linecount=0;
QStringList list;
QTime dieTime;
Boincmanager::Boincmanager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Boincmanager)
{
    ui->setupUi(this);
    if(!((QDir("BOINC").exists())&&(QFile("BOINC/boinccmd").exists())))
    {
        Boincdialog *d = new Boincdialog(this);
        d->show();
    }
   else{
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
    system("rm version.txt");}
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

void *executor(void* param)
{
    system(cmd.toUtf8().constData());
    done=true;
}

void *updater(void* param)
{
    QFile file("started.txt");
    int i=0;
    file.open (QIODevice::ReadOnly);
    line="";
    QTextStream stream ( &file );
    while(i<linecount){
    stream.readLine();
    i++;}
    while( !stream.atEnd()) {
        line += stream.readLine()+"\n";
        linecount++;
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
    if(!((QDir("BOINC").exists())&&(QFile("BOINC/boinc").exists())))
    {
        Boincdialog *d = new Boincdialog(this);
        d->show();
    }
    else{
    started=true;
    linecount=0;
    pthread_attr_init (&attr1);
    pthread_create(&tid1,&attr1,runner,NULL);
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);
    ui->projectext->setEnabled(true);
    ui->textEdit->setText("BOINC started\nLoading...");
    delay(2);
    ui->textEdit->setText("");
    ui->tab_2->setEnabled(true);
    ui->tabWidget_2->setEnabled(true);
    ui->createac->setEnabled(true);
    ui->tab_3->setEnabled(true);
    ui->tab_4->setEnabled(true);
       while(ui->tabWidget->currentIndex()==0 && started){
       pthread_attr_init(&attr2);
       pthread_create(&tid2,&attr2,updater,NULL);
       delay(1);
       if(!(line.compare("")==0))
          ui->textEdit->append(line);
          delay(4);}
}}


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
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,updater,NULL);
    delay(1);
       ui->textEdit->append(line);
}



void Boincmanager::on_tabWidget_currentChanged(int index)
{
    ui->projectext->setText("");
    if(index==0){
        while(ui->tabWidget->currentIndex()==0 && started){
        pthread_attr_init(&attr2);
        pthread_create(&tid2,&attr2,updater,NULL);
        delay(1);
        if(!(line.compare("")==0))
           ui->textEdit->append(line);
           delay(4);
        }
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
                line+="\n\nPROJECT URL\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\n\nUSER NAME\t\t: ";
                line+=list.at(1);
                statstream.readLine();statstream.readLine();
                list=statstream.readLine().split(": ");
                line+="\n\nUSER TOTAL CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\n\nUSER AVG. CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\n\nHOST TOTAL CREDIT\t\t: ";
                line+=list.at(1);
                list=statstream.readLine().split(": ");
                line+="\n\nHOST AVG. CREDIT\t\t: ";
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
    cmd = cmd1+cmd2+cmd3+cmd4+cmd5+cmd6;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Creating account...");
    while(!done)
    delay(3);
    QFile file("created.txt");
    file.open (QIODevice::ReadOnly);
    QTextStream stream ( &file );
    line="";
        while(!stream.atEnd()){
        temp=stream.readLine();
    if(temp.startsWith("account key"))
    {
        list=temp.split(": ");
        line="Account Key : ";
        line+=list.at(1);
        line+="\n\n Note down the account key for getting access to your account later";
        ui->auth->setText(list.at(1));
        break;
    }
    else
        line="Account creation failed!";
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
    cmd = cmd1+cmd2+cmd3;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
     ui->projectext->setText("Attaching...");
     delay(2);
    line="PROJECT ATTACHED SUCCESSFULLY";
    ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_detach_clicked()
{
    if(started){
   QString cmd1 = "./BOINC/boinccmd --project ";
   QString cmd2 = ui->prurl2->text();
   QString cmd3 = " detach";
   cmd = cmd1+cmd2+cmd3;
   pthread_attr_init(&attr2);
   pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Detaching project...");
    delay(2);
        line="PROJECT DETACHED SUCCESFULLY";
        ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_reset_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" reset";
    cmd=cmd1+cmd2+cmd3;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Resetting project...");
    delay(2);
        line="RESETTED PROJECT SUCCESFULLY";
        ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_update_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" update";
    cmd=cmd1+cmd2+cmd3;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Updating project...");
    delay(2);
        line="PROJECT UPDATED SUCCESSFULLY";
        ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_resume_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" resume";
    cmd=cmd1+cmd2+cmd3;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Resuming project...");
    delay(2);
        line="PROJECT RESUMED SUCCESSFULLY";
        ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_suspend_clicked()
{
    if(started){
    QString cmd1="./BOINC/boinccmd --project ";
    QString cmd2=ui->prurl3->text();
    QString cmd3=" suspend";
    cmd=cmd1+cmd2+cmd3;
    pthread_attr_init(&attr2);
    pthread_create(&tid2,&attr2,executor,NULL);
    ui->projectext->setText("Suspending project...");
    delay(2);
        line="PROJECT SUSPENDED SUCCESSFULLY";
        ui->projectext->setText(line);
    }
    else
        ui->projectext->setText("BOINC is not started.\nStart the BOINC first.");
}

void Boincmanager::on_tabWidget_2_currentChanged(int index)
{
    ui->prurl1->setText("");
    ui->prurl2->setText("");
    ui->prurl3->setText("");
    ui->email->setText("");
    ui->uname->setText("");
    ui->pwd->setText("");

}
