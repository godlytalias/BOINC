#include "boincmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Boincmanager w;
    w.show();

    return a.exec();
}
