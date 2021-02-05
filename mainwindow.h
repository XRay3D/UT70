#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ut70x.h"

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked(bool checked);

private:
    Ui::MainWindow* ui;
    UT70X ut;
};
#endif // MAINWINDOW_H
