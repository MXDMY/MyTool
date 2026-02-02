#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QWidget>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QListWidget>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QTimer>
#include <QTextCodec>

namespace Ui {
class SerialPort;
}

class SerialPort : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPort(QWidget *parent = nullptr);
    ~SerialPort();

public:
    enum Encoding
    {
        Unicode,
        UTF8,
    };
    Q_ENUM(Encoding)

protected:
    void closeEvent(QCloseEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_open_btn_clicked();

    void on_sche_send_check_stateChanged(int arg1);

    void on_msg_clear_btn_clicked();

    void sche_send_timer_timeoutSlot();

    void on_send_btn_clicked();

    void on_port_area_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_clear_size_btn_clicked();

    void on_clear_btn_clicked();

private:
    void setFixInfo(QSerialPortInfo info);

    void setVarInfo(QSerialPort* port);

    // 根据端口名获取端口在对应列表的索引
    int getPortIndex(QString port_name);

    void appendMessage(QString msg);

    void send(QString text);

private:
    Ui::SerialPort *ui;

    /*
     * port_area 的内容与 info_list、port_list 的内容严格对应
     * 其中，info_list 与 port_list 顺序严格一致
    */
    QList<QSerialPortInfo> info_list;
    QList<QSerialPort*> port_list;

    QTimer* sche_send_timer = Q_NULLPTR;

    qulonglong send_size = 0;
    qulonglong recv_size = 0;
};

#endif // SERIALPORT_H
