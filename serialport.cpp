#include "serialport.h"
#include "ui_serialport.h"

SerialPort::SerialPort(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialPort)
{
    ui->setupUi(this);

    setWindowTitle(tr("串口工具"));

    info_list = QSerialPortInfo::availablePorts();
    for (QSerialPortInfo info : info_list)
    {
        ui->port_area->addItem(info.portName());
        port_list.append(new QSerialPort(info, this));
    }

    ui->terminal_area->setLineWrapMode(QTextEdit::NoWrap);
    ui->msg_area->setLineWrapMode(QTextEdit::NoWrap);
    ui->msg_area->setToolTip(tr("消息窗"));
    ui->port_area->installEventFilter(this);

    sche_send_timer = new QTimer(this);
    connect(sche_send_timer, &QTimer::timeout, this, &SerialPort::sche_send_timer_timeoutSlot);

    // FIXINFO 设置
    ui->description->setText(tr("描述："));
    ui->manufacturer->setText(tr("制造商："));
    ui->port_name->setText(tr("端口名："));
    ui->product_identifier->setText(tr("产品标识符："));
    ui->serial_number->setText(tr("序列号："));
    ui->system_location->setText(tr("系统位置："));
    ui->vendor_identifier->setText(tr("供应商标识符："));

    // VARINFO 设置
    ui->baudrate_label->setText(tr("波特率："));
    ui->databit_label->setText(tr("数据位："));
    ui->stopbit_label->setText(tr("停止位："));
    ui->parity_label->setText(tr("校验位："));
    ui->flowctrl_label->setText(tr("流控制："));
    QRegularExpression re("^[1-9][0-9]*$");
    QRegularExpressionValidator* rev = new QRegularExpressionValidator(re, ui->baudrate);
    ui->baudrate->setValidator(rev);
    ui->databit->addItem("Unknown", QSerialPort::UnknownDataBits);
    ui->databit->addItem("5", QSerialPort::Data5);
    ui->databit->addItem("6", QSerialPort::Data6);
    ui->databit->addItem("7", QSerialPort::Data7);
    ui->databit->addItem("8", QSerialPort::Data8);
    ui->stopbit->addItem("Unknown", QSerialPort::UnknownStopBits);
    ui->stopbit->addItem("1", QSerialPort::OneStop);
    ui->stopbit->addItem("1.5", QSerialPort::OneAndHalfStop);
    ui->stopbit->addItem("2", QSerialPort::TwoStop);
    ui->parity->addItem("Unknown", QSerialPort::UnknownParity);
    ui->parity->addItem("No", QSerialPort::NoParity);
    ui->parity->addItem("Odd", QSerialPort::OddParity);
    ui->parity->addItem("Even", QSerialPort::EvenParity);
    ui->parity->addItem("Space", QSerialPort::SpaceParity);
    ui->parity->addItem("Mark", QSerialPort::MarkParity);
    ui->flowctrl->addItem("Unknown", QSerialPort::UnknownFlowControl);
    ui->flowctrl->addItem("No", QSerialPort::NoFlowControl);
    ui->flowctrl->addItem("RTS/CTS", QSerialPort::HardwareControl);
    ui->flowctrl->addItem("XON/XOFF", QSerialPort::SoftwareControl);

    // OPS 设置
    ui->open_btn->setText(tr("打开"));
    ui->clear_btn->setText(tr("清除终端"));
    ui->msg_clear_btn->setText(tr("清除消息"));
    ui->send_btn->setText(tr("发送"));
    ui->timestamp_check->setText(tr("时间戳|前缀"));
    ui->hex_show_check->setText(tr("HEX 显示"));
    ui->hex_send_check->setText(tr("HEX 发送"));
    ui->sche_send_check->setText(tr("定时发送"));
    ui->sche_send_interval->setPlaceholderText(tr("ms / 次"));
    rev = new QRegularExpressionValidator(re, ui->sche_send_interval);
    ui->sche_send_interval->setValidator(rev);
    ui->data_area->setAcceptRichText(false);
    ui->encoding_label->setText(tr("字符编码"));
    ui->encoding_box->addItem("UTF-8", UTF8);
    ui->encoding_box->addItem("Unicode(UTF-16)", Unicode);
    ui->clear_size_btn->setText(tr("清除字节数"));
    ui->send_size_label->setToolTip(tr("已发送"));
    ui->recv_size_label->setToolTip(tr("已接收"));
}

SerialPort::~SerialPort()
{
    delete ui;
}

void SerialPort::closeEvent(QCloseEvent *event)
{
    for (QSerialPort* port : port_list)
    {
        if (port->isOpen())
        {
            port->clear();
            port->close();
        }
    }

    QWidget::closeEvent(event);
}

bool SerialPort::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void SerialPort::setFixInfo(QSerialPortInfo info)
{
    ui->description->setText(tr("描述：") + info.description());
    ui->manufacturer->setText(tr("制造商：") + info.manufacturer());
    ui->port_name->setText(tr("端口名：") + info.portName());
    ui->product_identifier->setText(tr("产品标识符：") + "0x" + QString::number(info.productIdentifier(), 16));
    ui->serial_number->setText(tr("序列号：") + info.serialNumber());
    ui->system_location->setText(tr("系统位置：") + info.systemLocation());
    ui->vendor_identifier->setText(tr("供应商标识符：") + "0x" + QString::number(info.vendorIdentifier(), 16));
}

void SerialPort::setVarInfo(QSerialPort* port)
{
    // 波特率
    ui->baudrate->setText(QString::number(port->baudRate()));
    // 数据位
    for (int i = 0; i < ui->databit->count(); i++)
    {
        if (ui->databit->itemData(i) == port->dataBits())
        {
            ui->databit->setCurrentIndex(i);
            break;
        }
    }
    // 停止位
    for (int i = 0; i < ui->stopbit->count(); i++)
    {
        if (ui->stopbit->itemData(i) == port->stopBits())
        {
            ui->stopbit->setCurrentIndex(i);
            break;
        }
    }
    // 校验位
    for (int i = 0; i < ui->parity->count(); i++)
    {
        if (ui->parity->itemData(i) == port->parity())
        {
            ui->parity->setCurrentIndex(i);
            break;
        }
    }
    // 流控制
    for (int i = 0; i < ui->flowctrl->count(); i++)
    {
        if (ui->flowctrl->itemData(i) == port->flowControl())
        {
            ui->flowctrl->setCurrentIndex(i);
            break;
        }
    }
}

int SerialPort::getPortIndex(QString port_name)
{
    for (int i = 0; i < info_list.count(); i++)
    {
        if (info_list.at(i).portName() == port_name)
        {
            return i;
        }
    }
    return -1;
}

void SerialPort::appendMessage(QString msg)
{
    ui->msg_area->append(
            QDateTime::currentDateTime().toString("[yyyy.MM.dd HH:mm:ss:zzz] ") + msg);
}

void SerialPort::send(QString text)
{
    if (Q_NULLPTR == ui->port_area->currentItem())
    {
        appendMessage(tr("无效操作项"));
        return;
    }
    QString port_name = ui->port_area->currentItem()->text();
    int port_index = getPortIndex(port_name);
    if (port_index < 0)
    {
        appendMessage(tr("无效的端口名"));
        return;
    }
    QSerialPort* port = port_list.at(port_index);
    if (! port->isOpen())
    {
        appendMessage(tr("端口未打开"));
        return;
    }

    // 发送数据
    QString terminal_text;
    if (Qt::Unchecked == ui->hex_send_check->checkState())
    {
        const char* name = Q_NULLPTR;
        switch (ui->encoding_box->currentData().toInt())
        {
        case Unicode:
            name = "UTF-16";
            break;
        case UTF8:
            name = "UTF-8";
            break;
        default:
            appendMessage(tr("无法处理的字符编码"));
            return;
        }
        QTextCodec* codec = QTextCodec::codecForName(name);
        QByteArray ba = codec->fromUnicode(text);
        qint64 size = port->write(ba);
        terminal_text = codec->toUnicode(ba.left(size));

        send_size += size;
    }
    else
    {
        // TODO
    }
    // 显示已发送字节数
    ui->send_size_label->setText(QString::number(send_size));
    // 回显已发送的数据
    if (Qt::Unchecked != ui->timestamp_check->checkState())
    {
        terminal_text = QDateTime::currentDateTime().toString("[HH:mm:ss:zzz]->")
                + port_name + "$" + terminal_text;
    }
    ui->terminal_area->append(terminal_text);
}

void SerialPort::on_open_btn_clicked()
{
    if (Q_NULLPTR == ui->port_area->currentItem())
    {
        appendMessage(tr("无效操作项"));
        return;
    }
    QString port_name = ui->port_area->currentItem()->text();
    int port_index = getPortIndex(port_name);
    if (port_index < 0)
    {
        appendMessage(tr("无效的端口名"));
        return;
    }
    QSerialPort* port = port_list.at(port_index);

    if (port->isOpen())
    {
        port->close();
        ui->open_btn->setText(tr("打开"));
        ui->port_area->setEnabled(true);
    }
    else
    {
        port->setBaudRate(ui->baudrate->text().toInt());
        port->setDataBits((QSerialPort::DataBits)ui->databit->currentData().toInt());
        port->setStopBits((QSerialPort::StopBits)ui->stopbit->currentData().toInt());
        port->setParity((QSerialPort::Parity)ui->parity->currentData().toInt());
        port->setFlowControl((QSerialPort::FlowControl)ui->flowctrl->currentData().toInt());

        if (port->open(QIODevice::ReadWrite))
        {
            ui->open_btn->setText(tr("关闭"));
            ui->port_area->setEnabled(false);
        }
        else
            appendMessage(tr("打开失败"));
    }
}

void SerialPort::on_sche_send_check_stateChanged(int arg1)
{
    if (Qt::Unchecked == arg1)
    {
        sche_send_timer->stop();
    }
    else
    {
        if (ui->sche_send_interval->text().isEmpty())
        {
            appendMessage(tr("请输入发送间隔"));
            // 会再次触发 stateChanged 信号，但无大碍
            ui->sche_send_check->setCheckState(Qt::Unchecked);
            return;
        }

        sche_send_timer->start(ui->sche_send_interval->text().toInt());
    }
}

void SerialPort::on_msg_clear_btn_clicked()
{
    ui->msg_area->clear();
}

void SerialPort::sche_send_timer_timeoutSlot()
{
    QString text = ui->data_area->toPlainText();
    if (text.isEmpty())
        return;

    send(text);
}

void SerialPort::on_send_btn_clicked()
{
    QString text = ui->data_area->toPlainText();
    if (text.isEmpty())
        return;

    send(text);
}

void SerialPort::on_port_area_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    (void)previous;
    int index = getPortIndex(current->text());
    QSerialPortInfo info = info_list.at(index);
    QSerialPort* port = port_list.at(index);

    setFixInfo(info);
    setVarInfo(port);
    // set ops info
    if (port->isOpen())
        ui->open_btn->setText(tr("关闭"));
    else
        ui->open_btn->setText(tr("打开"));
}

void SerialPort::on_clear_size_btn_clicked()
{
    send_size = 0;
    recv_size = 0;
    ui->send_size_label->setText("0");
    ui->recv_size_label->setText("0");
}

void SerialPort::on_clear_btn_clicked()
{
    ui->terminal_area->clear();
}
