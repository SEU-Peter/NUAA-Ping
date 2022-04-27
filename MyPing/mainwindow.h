#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "Ping.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
    public slots:
        void on_PingButton1_clicked();
        void Ping1Result(Ping);
        void on_PingButton2_clicked();
};

class PingThread : public QThread
{
    Q_OBJECT
    public:
        PingThread(char addr[100]);
    signals:
        void isDone(Ping);  //处理完成信号
    protected:
        char addr[100];
        void run();//通过start()间接调用

};

#endif // MAINWINDOW_H
