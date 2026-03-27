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

    settings = new QSettings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat, this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    QSize mainwindow_size = settings->value(key_winsize, QSize(640, 360)).toSize();
    resize(mainwindow_size);
    QPoint central_pos(QApplication::desktop()->width() / 2 - mainwindow_size.width() / 2
                        , QApplication::desktop()->height() / 2 - mainwindow_size.height() / 2);
    move(settings->value(key_winpos, central_pos).toPoint());

    // 创建场景、视图和视频图元
    QGraphicsScene* scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->viewport()->installEventFilter(this);
    view->viewport()->setMouseTracking(true);
    view->setStyleSheet("border: 1px solid #889cfc;");
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
    proxy_close_btn = scene->addWidget(close_btn);
    proxy_close_btn->setPos(mainwindow_size.width() - 32 - 10, 10);

    QPushButton* folder_btn = new QPushButton(QIcon(":/image/ico/folder.png"), "");
    connect(folder_btn, &QPushButton::clicked, this, &MainWindow::folder_btn_clickedSlot);
    folder_btn->setStyleSheet(btn_style);
    proxy_folder_btn = scene->addWidget(folder_btn);
    proxy_folder_btn->setPos(10, 10);

    QPushButton* serial_port_btn = new QPushButton(QIcon(":/image/ico/monitor.png"), "");
    connect(serial_port_btn, &QPushButton::clicked, this, &MainWindow::serial_port_btn_clickedSlot);
    serial_port_btn->setStyleSheet(btn_style);
    proxy_serial_port_btn = scene->addWidget(serial_port_btn);
    proxy_serial_port_btn->setPos(10, 52);

    QPushButton* blue_tooth_btn = new QPushButton(QIcon(":/image/ico/signal.png"), "");
    connect(blue_tooth_btn, &QPushButton::clicked, this, &MainWindow::blue_tooth_btn_clickedSlot);
    blue_tooth_btn->setStyleSheet(btn_style);
    proxy_blue_tooth_btn = scene->addWidget(blue_tooth_btn);
    proxy_blue_tooth_btn->setPos(10, 94);
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
    static QPoint drag_pos;    // 记录拖动起始坐标
    static QPoint stretch_pos; // 记录拉伸起始坐标
    static bool allow_drag = false;
    static bool is_stretch_area = false;
    static bool allow_stretch = false;
    static StretchType stretch_type;

    if (qobject_cast<QWidget*>(watched) == view->viewport())
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            if (is_stretch_area)
            {
                stretch_pos = mouse_event->globalPos();
                allow_stretch = true;
            }
            else
            {
                drag_pos = mouse_event->globalPos();
                allow_drag = true;
            }
        }
            break;
        case QEvent::MouseMove:
        {
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            QPoint mouse_pos = mouse_event->globalPos();
            int mouse_x = mouse_pos.x();
            int mouse_y = mouse_pos.y();
            QPoint window_pos = pos();
            int window_x = window_pos.x();
            int window_y = window_pos.y();
            int window_rx = window_x + frameGeometry().width();
            int window_bx = window_x;
            int window_by = window_y + frameGeometry().height();
            int window_rbx = window_rx;
            int window_rby = window_by;
            int scope = 10;
            QSize window_size = size();

            if (allow_drag)
            {
                move(window_pos + mouse_pos - drag_pos);
                drag_pos = mouse_pos;
                break;
            }

            if (allow_stretch)
            {
                QPoint p = mouse_pos - stretch_pos;
                switch (stretch_type)
                {
                case TopLeft:
                    resize(window_size.width() - p.x(), window_size.height() - p.y());
                    move(window_pos + p);
                    break;
                case TopRight:
                    resize(window_size.width() + p.x(), window_size.height() - p.y());
                    move(window_pos.x(), window_pos.y() + p.y());
                    break;
                case ButtomLeft:
                    resize(window_size.width() - p.x(), window_size.height() + p.y());
                    move(window_pos.x() + p.x(), window_pos.y());
                    break;
                case ButtomRight:
                    resize(window_size.width() + p.x(), window_size.height() + p.y());
                    break;
                case Top:
                    resize(window_size.width(), window_size.height() - p.y());
                    move(window_pos.x(), window_pos.y() + p.y());
                    break;
                case Buttom:
                    resize(window_size.width(), window_size.height() + p.y());
                    break;
                case Left:
                    resize(window_size.width() - p.x(), window_size.height());
                    move(window_pos.x() + p.x(), window_pos.y());
                    break;
                case Right:
                    resize(window_size.width() + p.x(), window_size.height());
                    break;
                }
                stretch_pos = mouse_pos;
                break;
            }

            is_stretch_area = true; // 先假设在拉伸区域
            if (mouse_x >= window_x && mouse_x <= window_x + scope && mouse_y >= window_y && mouse_y <= window_y + scope)
            {
                view->viewport()->setCursor(Qt::SizeFDiagCursor);
                stretch_type = TopLeft;
            }
            else if (mouse_x >= window_rbx - scope && mouse_x <= window_rbx && mouse_y >= window_rby - scope && mouse_y <= window_rby)
            {
                view->viewport()->setCursor(Qt::SizeFDiagCursor);
                stretch_type = ButtomRight;
            }
            else if (mouse_x >= window_rx - scope && mouse_x <= window_rx && mouse_y >= window_y && mouse_y <= window_y + scope)
            {
                view->viewport()->setCursor(Qt::SizeBDiagCursor);
                stretch_type = TopRight;
            }
            else if (mouse_x >= window_bx && mouse_x <= window_bx + scope && mouse_y >= window_by - scope && mouse_y <= window_by)
            {
                view->viewport()->setCursor(Qt::SizeBDiagCursor);
                stretch_type = ButtomLeft;
            }
            else if (mouse_x >= window_x + scope && mouse_x <= window_rx - scope && mouse_y >= window_y && mouse_y <= window_y + scope)
            {
                view->viewport()->setCursor(Qt::SizeVerCursor);
                stretch_type = Top;
            }
            else if (mouse_x >= window_x + scope && mouse_x <= window_rx - scope && mouse_y >= window_by - scope && mouse_y <= window_by)
            {
                view->viewport()->setCursor(Qt::SizeVerCursor);
                stretch_type = Buttom;
            }
            else if (mouse_x >= window_x && mouse_x <= window_x + scope && mouse_y >= window_y + scope && mouse_y <= window_by - scope)
            {
                view->viewport()->setCursor(Qt::SizeHorCursor);
                stretch_type = Left;
            }
            else if (mouse_x >= window_rx - scope && mouse_x <= window_rx && mouse_y >= window_y + scope && mouse_y <= window_by - scope)
            {
                view->viewport()->setCursor(Qt::SizeHorCursor);
                stretch_type = Right;
            }
            else
            {
                view->viewport()->setCursor(Qt::ArrowCursor);
                is_stretch_area = false;
            }
        }
            break;
        case QEvent::MouseButtonRelease:
            allow_drag = false;
            allow_stretch = false;
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
    video_item->setSize(view->size());
    video_item->setPos(0, 0);
    proxy_close_btn->setPos(event->size().width() - 32 - 10, 10);
    proxy_folder_btn->setPos(10, 10);
    proxy_serial_port_btn->setPos(10, 52);
    proxy_blue_tooth_btn->setPos(10, 94);

    settings->setValue(key_winsize, event->size());

    QMainWindow::resizeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    settings->setValue(key_winpos, event->pos());

    QMainWindow::moveEvent(event);
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
    QString filepath = QFileDialog::getOpenFileName(this, tr("选择你的背景")
                                                    , settings->value(key_bgpath).toString());
    if (false == filepath.isEmpty())
        settings->setValue(key_bgpath, filepath);
}

void MainWindow::close_btn_clickedSlot()
{
    close();
}
