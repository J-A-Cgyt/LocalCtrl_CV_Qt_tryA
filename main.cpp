#include "CV_MinWin.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CV_MinWin w;
    w.show();
    return a.exec();
}
