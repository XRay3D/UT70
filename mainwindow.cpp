#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto pil { QSerialPortInfo::availablePorts() };

    std::sort(pil.begin(), pil.end(), [](auto&& pi1, auto&& pi2) {
        return pi1.portName().mid(3).toInt() < pi2.portName().mid(3).toInt();
    });

    for (auto&& portInfo : pil)
        ui->comboBox->addItem(portInfo.portName());

    connect(&ut, &UT70X::valueChanged, ui->doubleSpinBox, &QDoubleSpinBox::setValue);
    connect(&ut, &UT70X::unit, ui->doubleSpinBox, &QDoubleSpinBox::setSuffix);

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

void MainWindow::on_pushButton_clicked(bool /*checked*/)
{
    ut.initserial(ui->comboBox->currentText());
}
