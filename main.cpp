#include "mainwindow.h"
#include <QApplication>

#include "my_log.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyLogIns.installer_logger(new MyLogNS::ConsoleLogger());
    MyLogIns.is_show_function_name = false;


    MainWindow w;
    w.show();

    return a.exec();
}
