#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for (auto&& portInfo : QSerialPortInfo::availablePorts())
        ui->comboBox->addItem(portInfo.portName());

    QSettings settings;
    settings.beginGroup("MainWindow");
    ui->comboBox->setCurrentIndex(settings.value("comboBox").toInt());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("comboBox", ui->comboBox->currentIndex());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    delete ui;
}

void MainWindow::on_pushButton_clicked(bool checked)
{
    ut.initserial(ui->comboBox->currentText());
}

void MainWindow::on_pushButton_2_clicked(bool checked)
{
    ut.mmm();
}
