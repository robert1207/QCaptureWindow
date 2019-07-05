#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "./capturing_dialog/capturing_dialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_showButton_clicked();

    void on_closeButton_clicked();

private:
     CapturingDialog *capturing_dialog_;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
