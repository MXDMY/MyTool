#include "bluetooth.h"
#include "ui_bluetooth.h"

BlueTooth::BlueTooth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BlueTooth)
{
    ui->setupUi(this);

    setWindowTitle(tr("蓝牙工具"));

    ui->tabWidget->setTabText(0, tr("低功耗蓝牙"));

    QItemSelectionModel* m = ui->ble_dev_view->selectionModel();
    QStandardItemModel* model = new QStandardItemModel();
    ui->ble_dev_view->setModel(model);
    delete m;

    discovery = new QBluetoothDeviceDiscoveryAgent(this);
    discovery->setLowEnergyDiscoveryTimeout(0); // 对于 BLE 搜索永远不超时
    connect(discovery, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BlueTooth::discovery_deviceDiscoveredSlot);
    connect(discovery, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &BlueTooth::discovery_deviceUpdatedSlot);
    discovery->start();
}

BlueTooth::~BlueTooth()
{
    delete ui;
}

void BlueTooth::discovery_deviceDiscoveredSlot(const QBluetoothDeviceInfo& info)
{
    if (QBluetoothDeviceInfo::LowEnergyCoreConfiguration == info.coreConfigurations())
    {
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_dev_view->model());
        QStandardItem* item = new QStandardItem();
        item->setData(info.name(), Qt::DisplayRole);
        // TODO
#ifdef Q_OS_APPLE
#else
#endif

        item->setEditable(false);
        model->appendRow(item);
        model->setSortRole(Qt::DisplayRole);
        model->sort(0, Qt::DescendingOrder);
    }
    else
    {
        // TODO
    }
}

void BlueTooth::discovery_deviceUpdatedSlot(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields)
{
    (void)info;
    (void)updatedFields;
    // TODO
}
