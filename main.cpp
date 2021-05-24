#include "meteoeci_gui.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");
    MeteoECI_GUI w;
    w.show();

    return a.exec();
}
