#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
HWND workerw = NULL; // 第二个 WorkerW 窗口句柄
inline static WINBOOL CALLBACK EnumWindowsProc(HWND handle, LPARAM lparam)
{
    (void)lparam;
    // 获取第一个 WorkerW 窗口
    HWND defview = FindWindowEx(handle, 0, L"SHELLDLL_DefView", NULL);

    if (defview) // 找到第一个 WorkerW 窗口
    {
        // 获取第二个 WorkerW 窗口的窗口句柄
        workerw = FindWindowEx(0, handle, L"WorkerW", NULL);
    }
    return true;
}
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("我的工具"));
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    QSize mainwindow_size(640, 360);
    resize(mainwindow_size);

    // 创建场景、视图和视频图元
    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsView* view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(view);
    QGraphicsVideoItem* video_item = new QGraphicsVideoItem();
    video_item->setSize(mainwindow_size);
    scene->addItem(video_item);
    // 创建视频播放器
    QMediaPlayer* player = new QMediaPlayer(this);
    player->setVideoOutput(video_item);
    player->setMedia(QUrl::fromLocalFile("/home/my/下载/马达加斯加的企鹅.mp4"));
    player->play();
    // 添加按钮作为图元
    QString btn_style = R"(
                            QPushButton
                            {
                                background-color: rgba(152, 144, 245, 100);
                                border: none;
                                color: #ffffff;
                                border-radius: 5px;
                            }
                            QPushButton:hover
                            {
                                background-color: rgba(240, 239, 254, 100);
                            }
                            QPushButton:pressed
                            {
                                background-color: transparent;
                            }
                            )";
    QSize btn_size(70, 30);
    QPushButton* serial_port_btn = new QPushButton(tr("串口"));
    connect(serial_port_btn, &QPushButton::clicked, this, &MainWindow::serial_port_btn_clickedSlot);
    serial_port_btn->setStyleSheet(btn_style);
    serial_port_btn->setFixedSize(btn_size);
    QPushButton* blue_tooth_btn = new QPushButton(tr("蓝牙主机"));
    connect(blue_tooth_btn, &QPushButton::clicked, this, &MainWindow::blue_tooth_btn_clickedSlot);
    blue_tooth_btn->setStyleSheet(btn_style);
    blue_tooth_btn->setFixedSize(btn_size);
    QGraphicsProxyWidget* proxy_serial_port_btn = scene->addWidget(serial_port_btn);
    QGraphicsProxyWidget* proxy_blue_tooth_btn = scene->addWidget(blue_tooth_btn);
    proxy_serial_port_btn->setPos(0, 30);
    proxy_blue_tooth_btn->setPos(0, 60);
    // 调整图元层级
    video_item->setZValue(0);
    proxy_serial_port_btn->setZValue(1);
    proxy_blue_tooth_btn->setZValue(1);

#if defined(Q_OS_WIN) && defined(DDESKTOP_ENABLE)
    ddesktop = new QVideoWidget();
    ddesktop->setAttribute(Qt::WA_DeleteOnClose);
    ddesktop->setWindowFlags(Qt::FramelessWindowHint);
    ddesktop->setFixedSize(QApplication::desktop()->size());

    int window_result;
    HWND window_handle = FindWindow(L"Progman", NULL);
    SendMessageTimeout(window_handle, 0x052c, 0, 0, SMTO_NORMAL, 0x3e8, (PDWORD_PTR)(&window_result));
    // 枚举窗口
    EnumWindows(EnumWindowsProc, (LPARAM)NULL);
    // 隐藏第二个 WorkerW 窗口，当以 Progman 为父窗口时需要对其进行隐藏，不然程序窗口会被第二个 WorkerW 覆盖
    ShowWindow(workerw, SW_HIDE);
    // SendMessageTimeout 后 window_handle 被置 0，需要重新获取一次
    window_handle = FindWindow(L"Progman", NULL);
    SetParent((HWND)ddesktop->winId(), window_handle);

    ddesktop->show();
    ddesktop_mplayer = new QMediaPlayer(this);
    connect(ddesktop_mplayer, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::mediaStatusChangedSlot);
    ddesktop_mplayer->setVideoOutput(ddesktop);
    ddesktop_mplayer->setMedia(QUrl::fromLocalFile("E:\\a\\马达加斯加的企鹅.1080p.国英双语.BD中英双字[最新电影www.dygangs.me].mp4"));
    ddesktop_mplayer->play();
    ddesktop_mplayer->setVolume(0);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ddesktop_mplayer)
        ddesktop_mplayer->stop();
    if (ddesktop)
        ddesktop->close();
    serial_port.close();
    blue_tooth.close();

    QMainWindow::closeEvent(event);
}

void MainWindow::serial_port_btn_clickedSlot()
{
    serial_port.show();
}

void MainWindow::mediaStatusChangedSlot(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
    case QMediaPlayer::EndOfMedia:
        if (ddesktop_mplayer && ddesktop)
            ddesktop_mplayer->play();
        break;
    default:
        break;
    }
}

void MainWindow::blue_tooth_btn_clickedSlot()
{
    blue_tooth.show();
}
