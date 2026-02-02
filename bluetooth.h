#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QWidget>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QStandardItemModel>
#include <QStandardItem>

namespace Ui {
class BlueTooth;
}

class BlueTooth : public QWidget
{
    Q_OBJECT

public:
    explicit BlueTooth(QWidget *parent = nullptr);
    ~BlueTooth();

private slots:
    void discovery_deviceDiscoveredSlot(const QBluetoothDeviceInfo &info);

    void discovery_deviceUpdatedSlot(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);

private:
    Ui::BlueTooth *ui;

    QBluetoothDeviceDiscoveryAgent* discovery = Q_NULLPTR;
    const int addr_uuid_role = Qt::UserRole;
    const int name_role = Qt::UserRole + 1;
    const int major_dev_class_role = Qt::UserRole + 2;
    const int minor_dev_class_role = Qt::UserRole + 3;
    const int serv_class = Qt::UserRole + 4;
};

#endif // BLUETOOTH_H
