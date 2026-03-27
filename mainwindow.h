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
#include <QFileDialog>
#include <QCoreApplication>
#include <QSettings>
#include <QApplication>

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

    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    enum StretchType
    {
        Top,
        Buttom,
        Left,
        Right,
        TopLeft,
        TopRight,
        ButtomLeft,
        ButtomRight
    };
    Q_ENUM(StretchType)

protected:
    void closeEvent(QCloseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void moveEvent(QMoveEvent *event) override;

private slots:
    void serial_port_btn_clickedSlot();

    void mediaStatusChangedSlot(QMediaPlayer::MediaStatus status);

    void blue_tooth_btn_clickedSlot();

    void folder_btn_clickedSlot();

    void close_btn_clickedSlot();

private:
    Ui::MainWindow *ui;

    QVideoWidget* ddesktop = Q_NULLPTR;
    QMediaPlayer* ddesktop_mplayer = Q_NULLPTR;

    SerialPort serial_port;
    BlueTooth blue_tooth;

    QGraphicsView* view = Q_NULLPTR;
    QGraphicsVideoItem* video_item = Q_NULLPTR;

    QSettings* settings = Q_NULLPTR;
    const char* key_bgpath = "video file path use by backgroud";
    const char* key_winsize = "mainwindow size";
    const char* key_winpos = "mainwindow pos";

    QGraphicsProxyWidget* proxy_close_btn = Q_NULLPTR;
    QGraphicsProxyWidget* proxy_folder_btn = Q_NULLPTR;
    QGraphicsProxyWidget* proxy_serial_port_btn = Q_NULLPTR;
    QGraphicsProxyWidget* proxy_blue_tooth_btn = Q_NULLPTR;
};
#endif // MAINWINDOW_H
