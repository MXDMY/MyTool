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
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    QSize mainwindow_size(640, 360);
    resize(mainwindow_size);

    settings = new QSettings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat, this);

    // 创建场景、视图和视频图元
    QGraphicsScene* scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->viewport()->installEventFilter(this);
    view->viewport()->setMouseTracking(true);
    setCentralWidget(view);
    video_item = new QGraphicsVideoItem();
    video_item->setSize(mainwindow_size);
    scene->addItem(video_item);
    // 创建视频播放器
    QMediaPlayer* player = new QMediaPlayer(this);
    player->setVideoOutput(video_item);
    player->setMedia(QUrl::fromLocalFile(settings->value(key_bgpath).toString()));
    player->play();
    // 添加按钮作为图元
    QString btn_style = R"(
                            QPushButton
                            {
                                background-color: rgba(152, 144, 245, 0);
                                border: none;
                                color: #ffffff;
                                border-radius: 5px;
                                icon-size: 32px;
                                width: 32px;
                                height: 32px;
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

    QPushButton* close_btn = new QPushButton(QIcon(":/image/ico/close.png"), "");
    connect(close_btn, &QPushButton::clicked, this, &MainWindow::close_btn_clickedSlot);
    close_btn->setStyleSheet(btn_style);
    QGraphicsProxyWidget* proxy_close_btn = scene->addWidget(close_btn);
    proxy_close_btn->setPos(mainwindow_size.width() - 32 - 5, 0);

    QPushButton* folder_btn = new QPushButton(QIcon(":/image/ico/folder.png"), "");
    connect(folder_btn, &QPushButton::clicked, this, &MainWindow::folder_btn_clickedSlot);
    folder_btn->setStyleSheet(btn_style);
    QGraphicsProxyWidget* proxy_folder_btn = scene->addWidget(folder_btn);
    proxy_folder_btn->setPos(5, 0);

    QPushButton* serial_port_btn = new QPushButton(QIcon(":/image/ico/monitor.png"), "");
    connect(serial_port_btn, &QPushButton::clicked, this, &MainWindow::serial_port_btn_clickedSlot);
    serial_port_btn->setStyleSheet(btn_style);
    QGraphicsProxyWidget* proxy_serial_port_btn = scene->addWidget(serial_port_btn);
    proxy_serial_port_btn->setPos(5, 32);

    QPushButton* blue_tooth_btn = new QPushButton(QIcon(":/image/ico/signal.png"), "");
    connect(blue_tooth_btn, &QPushButton::clicked, this, &MainWindow::blue_tooth_btn_clickedSlot);
    blue_tooth_btn->setStyleSheet(btn_style);
    QGraphicsProxyWidget* proxy_blue_tooth_btn = scene->addWidget(blue_tooth_btn);
    proxy_blue_tooth_btn->setPos(5, 64);
    // 调整图元层级
    // video_item->setZValue(0);
    // proxy_serial_port_btn->setZValue(1);
    // proxy_blue_tooth_btn->setZValue(1);

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
    ddesktop_mplayer->setMedia(QUrl::fromLocalFile(settings->value(key_bgpath).toString()));
    ddesktop_mplayer->play();
    ddesktop_mplayer->setVolume(0);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint drag_pos; // 记录拖动起始坐标
    static bool allow_drag = false;

    if (qobject_cast<QWidget*>(watched) == view->viewport())
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
            if (false == allow_drag)
            {
                QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
                drag_pos = mouse_event->globalPos();
                allow_drag = true;
            }
            break;
        case QEvent::MouseMove:
            if (allow_drag)
            {
                QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
                move(pos() + mouse_event->globalPos() - drag_pos);
                drag_pos = mouse_event->globalPos();
            }
            break;
        case QEvent::MouseButtonRelease:
            allow_drag = false;
            break;
        default:
            break;
        }
    }

    return QMainWindow::eventFilter(watched, event);
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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (video_item && view)
    {
        video_item->setSize(view->size());
        video_item->setPos(0, 0);
    }
    QMainWindow::resizeEvent(event);
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

void MainWindow::folder_btn_clickedSlot()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("选择你的背景"));
    if (false == filepath.isEmpty())
        settings->setValue(key_bgpath, filepath);
}

void MainWindow::close_btn_clickedSlot()
{
    close();
}
