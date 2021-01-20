#include "mainwindow.h"

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("GGEasy");
    QApplication::setOrganizationName("XrSoft");
    QApplication::setApplicationVersion("0.1");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationDirPath());

    MainWindow w;
    w.show();
    return a.exec();
}
