#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QDesktopWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "serialport.h"
#include "bluetooth.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_serial_port_btn_clicked();

    void mediaStatusChangedSlot(QMediaPlayer::MediaStatus status);

    void on_blue_tooth_btn_clicked();

private:
    Ui::MainWindow *ui;

    QVideoWidget* ddesktop = Q_NULLPTR;
    QMediaPlayer* ddesktop_mplayer = Q_NULLPTR;

    SerialPort serial_port;
    BlueTooth blue_tooth;
};
#endif // MAINWINDOW_H
