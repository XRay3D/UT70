#include "mainwindow.h"

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug\033[0m: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info\033[0m: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning\033[0m: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical\033[0m: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal\033[0m: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}

int main(int argc, char* argv[])
{
    //    printf("\033[1;47;35m==%d==\033[0m\n", 1);mbg
    //    printf("\033[2;47;35m==%d==\033[0m\n", 2);mg
    //    printf("\033[3;100;30m==%d==\033[0m\n", 3);
    //    printf("\033[3;104;30m==%d==\033[0m\n", 4);
    //    printf("\033[3;42;30m==%d==\033[0m\n", 5);
    //    printf("\033[3;43;30m==%d==\033[0m\n", 6);
    //    printf("\033[3;44;30m==%d==\033[0m\n", 7);
    //    printf("\033[3;47;35m==%d==\033[0m\n", 8);
    //    printf("\x1B[31m==%d==\033[0m\n", 9);
    //    printf("\x1B[32m==%d==\033[0m\n", 10);
    //    printf("\x1B[33m==%d==\033[0m\n", 11);
    //    printf("\x1B[34m==%d==\033[0m\n", 12);
    //    printf("\x1B[35m==%d==\033[0m\n", 13);
    //    printf("\x1B[36m==%d==\033[0m\n", 14);
    //    printf("\x1B[36m==%d==\033[0m\n", 15);
    //    printf("\x1B[36m==%d==\033[0m\n", 16);b
    //    printf("\x1B[37m==%d==\033[0m\n", 17);g
    //    printf("\x1B[93m==%d==\033[0m\n", 18);b

    //    qInstallMessageHandler(myMessageOutput);
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
