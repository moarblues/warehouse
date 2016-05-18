#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator ruTrans;
    ruTrans.load(QLocale(QLocale::Russian),"qt_ru","",QDir::currentPath().append("/trans"),".qm");
    a.installTranslator(&ruTrans);

    mainWindow w;
    w.show();
    
    return a.exec();
}
