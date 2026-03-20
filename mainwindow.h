#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QGraphicsProxyWidget>
#include <QPushButton>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "common.h"
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
    void serial_port_btn_clickedSlot();

    void mediaStatusChangedSlot(QMediaPlayer::MediaStatus status);

    void blue_tooth_btn_clickedSlot();

private:
    Ui::MainWindow *ui;

    QVideoWidget* ddesktop = Q_NULLPTR;
    QMediaPlayer* ddesktop_mplayer = Q_NULLPTR;

    SerialPort serial_port;
    BlueTooth blue_tooth;
};
#endif // MAINWINDOW_H
