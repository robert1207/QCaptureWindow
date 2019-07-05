#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    capturing_dialog_ = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_showButton_clicked()
{
    if(capturing_dialog_ != nullptr ) {

           capturing_dialog_->show();
       } else {
           capturing_dialog_ = new CapturingDialog(this);
           capturing_dialog_->show();
       }
}

void MainWindow::on_closeButton_clicked()
{
    capturing_dialog_->close();
    delete capturing_dialog_;
    capturing_dialog_ = nullptr;
}
