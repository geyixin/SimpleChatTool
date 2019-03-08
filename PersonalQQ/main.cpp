#include "widget.h"
#include <QApplication>
#include "dialoglist.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Widget w;
//    w.show();

    Dialoglist dl;
    dl.show();




    return a.exec();
}
