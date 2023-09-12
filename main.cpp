#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setWindowIcon(QIcon(":/icon.ico"));
    QCoreApplication::setOrganizationName("Genavir");
    QCoreApplication::setApplicationName("SBE_Fake");
    MainWindow w;
    w.show();
    return a.exec();
}
