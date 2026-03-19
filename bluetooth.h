#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QWidget>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QMenu>
#include <QDateTime>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QTimer>
#include <QLowEnergyDescriptor>
#include <QTextCodec>

#include "common.h"

namespace Ui {
class BlueTooth;
}

class BlueTooth : public QWidget
{
    Q_OBJECT

public:
    explicit BlueTooth(QWidget *parent = nullptr);
    ~BlueTooth();

public:
    enum ViewColRole
    {
        NameUUIDColRole,
        TypeColRole,
        PermissionColRole,
        ValueColRole
    };
    Q_ENUM(ViewColRole)

private slots:
    void discovery_deviceDiscoveredSlot(const QBluetoothDeviceInfo &info);

    void discovery_deviceUpdatedSlot(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);

    void on_ble_dev_view_clicked(const QModelIndex &index);

    void ble_dev_view_customContextMenuRequestedSlot(const QPoint &pos);

    void ble_search_act_triggeredSlot(bool checked = false);

    void ble_connect_act_triggeredSlot(bool checked = false);

    void ble_connectedSlot();

    void ble_connectionUpdatedSlot(const QLowEnergyConnectionParameters &newParameters);

    void ble_disconnectedSlot();

    void ble_discoveryFinishedSlot();

    void ble_errorSlot(QLowEnergyController::Error newError);

    void ble_serviceDiscoveredSlot(const QBluetoothUuid &newService);

    void ble_stateChangedSlot(QLowEnergyController::ControllerState state);

    void ble_serv_detail_timer_timeoutSlot();

    void on_ble_disconnect_btn_clicked();

    void on_ble_read_btn_clicked();

    void on_ble_write_btn_clicked();

    void ble_serv_characteristicChangedSlot(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

    void ble_serv_characteristicReadSlot(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);

    void ble_serv_characteristicWrittenSlot(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

    void ble_serv_descriptorReadSlot(const QLowEnergyDescriptor &descriptor, const QByteArray &value);

    void ble_serv_descriptorWrittenSlot(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);

    void ble_serv_errorSlot(QLowEnergyService::ServiceError newError);

private:
    void appendBleMessage(QString msg);

    void bleFree();

    void bleServViewHorHeadInit();

    // 将 ble 特征值权限显示为文本
    QString bleCharaPerm2Str(QLowEnergyCharacteristic::PropertyTypes p);

    // 根据 uuid 查找 ble 服务，没有返回空
    QLowEnergyService* bleGetServ(QBluetoothUuid uuid);

private:
    Ui::BlueTooth *ui;

    QBluetoothDeviceDiscoveryAgent* discovery = Q_NULLPTR;
    // 下方用于（经典/低功耗）设备视图的数据角色定义（这些角色负责描述一个完整的设备）
    const int addr_uuid_role = Qt::UserRole;
    const int name_role = Qt::UserRole + 1;
    const int major_dev_class_role = Qt::UserRole + 2;
    const int minor_dev_class_role = Qt::UserRole + 3;
    const int serv_class_role = Qt::UserRole + 4;

    // 下方用于 ble 设备的右键操作菜单
    QMenu* ble_dev_menu = Q_NULLPTR;
    QAction* ble_search_act = Q_NULLPTR;
    QAction* ble_connect_act = Q_NULLPTR;

    // 下方用于 ble 服务视图的数据角色定义
    const int ble_serv_name_role = Qt::UserRole + 5;
    const int ble_serv_uuid_role = Qt::UserRole + 6;
    const int ble_chara_name_role = Qt::UserRole + 7;
    const int ble_chara_uuid_role = Qt::UserRole + 8;
    const int ble_desc_name_role = Qt::UserRole + 9;
    const int ble_desc_uuid_role = Qt::UserRole + 10;
    // 下方用于 ble 服务视图的列标签角色定义
    const int ble_serv_view_col_role = Qt::UserRole + 11;

    // 当前 ble 连接使用的设备名与地址
    QString ble_dev_name;
    QString ble_dev_addr_uuid;

    // ble 断开连接时下方变量需要进行释放
    QLowEnergyController* ble = Q_NULLPTR;
    QList<QLowEnergyService*> ble_serv_list;
    int ble_serv_detail_index = 0; // 当前正在被列举特征值的服务索引
    QTimer* ble_serv_detail_timer = Q_NULLPTR;

};

#endif // BLUETOOTH_H
