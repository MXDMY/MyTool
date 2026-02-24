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

    ui->ble_dev_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ble_dev_view, &QListView::customContextMenuRequested, this
                , &BlueTooth::ble_dev_view_customContextMenuRequestedSlot);
    ble_search_act = new QAction(tr("搜索(BLE|经典)"));
    connect(ble_search_act, &QAction::triggered, this, &BlueTooth::ble_search_act_triggeredSlot);
    ble_connect_act = new QAction(tr("连接"));
    connect(ble_connect_act, &QAction::triggered, this, &BlueTooth::ble_connect_act_triggeredSlot);
    ble_dev_menu = new QMenu(ui->ble_dev_view);
    ble_dev_menu->addAction(ble_search_act);
    ble_dev_menu->addAction(ble_connect_act);

    discovery = new QBluetoothDeviceDiscoveryAgent(this);
    discovery->setLowEnergyDiscoveryTimeout(0); // 对于 BLE 搜索永远不超时
    connect(discovery, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BlueTooth::discovery_deviceDiscoveredSlot);
    connect(discovery, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &BlueTooth::discovery_deviceUpdatedSlot);

    m = ui->ble_serv_view->selectionModel();
    model = new QStandardItemModel();
    ui->ble_serv_view->setModel(model);
    delete m;
    bleServViewHorHeadInit();

    ui->addr_uuid_label->setText(tr("唯一标识符："));
    ui->name_label->setText(tr("设备名："));
    ui->dev_class_label->setText(tr("设备类型："));
    ui->serv_class_label->setText(tr("服务类型："));

    ui->ble_read_btn->setText(tr("读取"));
    ui->ble_write_btn->setText(tr("写入"));
    ui->ble_hex_send_check->setText(tr("HEX 发送"));
    ui->ble_disconnect_btn->setText(tr("断开"));
}

BlueTooth::~BlueTooth()
{
    delete ui;
}

void BlueTooth::discovery_deviceDiscoveredSlot(const QBluetoothDeviceInfo& info)
{
    QStandardItemModel* model;
    if (QBluetoothDeviceInfo::LowEnergyCoreConfiguration == info.coreConfigurations())
        model = qobject_cast<QStandardItemModel*>(ui->ble_dev_view->model());
    else
        return; // 经典蓝牙暂未实现，TODO

    // 避免重复添加
    for (int i = 0; i < model->rowCount(); i++)
    {
        QString addr_uuid = model->item(i)->data(addr_uuid_role).toString();
#ifdef Q_OS_APPLE
        if (addr_uuid == info.deviceUuid().toString())
            return;
#else
        if (addr_uuid == info.address().toString())
            return;
#endif
    }

    QStandardItem* item = new QStandardItem();
    item->setData(info.name(), Qt::DisplayRole);
#ifdef Q_OS_APPLE
    item->setData(info.deviceUuid().toString(), addr_uuid_role);
#else
    item->setData(info.address().toString(), addr_uuid_role);
#endif
    item->setData(info.name(), name_role);
    item->setData(info.majorDeviceClass(), major_dev_class_role);
    item->setData(info.minorDeviceClass(), minor_dev_class_role);
    item->setData(info.serviceClasses().operator unsigned int(), serv_class_role);

    item->setEditable(false);
    model->appendRow(item);
    model->setSortRole(Qt::DisplayRole);
    model->sort(0, Qt::DescendingOrder);
}

void BlueTooth::discovery_deviceUpdatedSlot(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields)
{
    (void)info;
    (void)updatedFields;
    // TODO
}

void BlueTooth::on_ble_dev_view_clicked(const QModelIndex &index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_dev_view->model());
    QStandardItem* item = model->itemFromIndex(index);

    QString addr_uuid = item->data(addr_uuid_role).toString();
    QString name = item->data(name_role).toString();
    quint32 major_dev_class = item->data(major_dev_class_role).toUInt();
    quint32 minor_dev_class = item->data(minor_dev_class_role).toUInt();
    quint32 serv_class = item->data(serv_class_role).toUInt();

    ui->addr_uuid_label->setText(tr("唯一标识符：") + addr_uuid);
    ui->name_label->setText(tr("设备名：") + name);
    ui->dev_class_label->setText(tr("设备类型：")
            + QString("[%1:%2]").arg(major_dev_class).arg(minor_dev_class));
    ui->serv_class_label->setText(tr("服务类型：") + QString::number(serv_class));
}

void BlueTooth::ble_dev_view_customContextMenuRequestedSlot(const QPoint &pos)
{
    ble_dev_menu->exec(ui->ble_dev_view->mapToGlobal(pos));
}

void BlueTooth::ble_search_act_triggeredSlot(bool checked)
{
    (void)checked;
    static bool is_search = false;

    if (is_search)
    {
        discovery->stop();
        ble_search_act->setText(tr("搜索(BLE|经典)"));
        is_search = false;
    }
    else
    {
        discovery->start();
        ble_search_act->setText(tr("停止搜索"));
        is_search = true;
    }
}

void BlueTooth::ble_connect_act_triggeredSlot(bool checked)
{
    (void)checked;

    if (ble)
    {
        appendBleMessage(tr("已有正在进行的连接，请先取消旧连接"));
        return;
    }

    QModelIndex index = ui->ble_dev_view->currentIndex();
    if (! index.isValid())
    {
        appendBleMessage(tr("无效的 BLE 设备视图索引"));
        return;
    }
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_dev_view->model());
    QStandardItem* item = model->itemFromIndex(index);
    QString addr_uuid = item->data(addr_uuid_role).toString();
    QString name = item->data(name_role).toString();
    quint32 major_dev_class = item->data(major_dev_class_role).toUInt();
    quint32 minor_dev_class = item->data(minor_dev_class_role).toUInt();
    quint32 serv_class = item->data(serv_class_role).toUInt();

#ifdef Q_OS_APPLE
    QBluetoothDeviceInfo info(QBluetoothUuid(addr_uuid), name
            , minor_dev_class << 2 | major_dev_class << 8 | serv_class << 13);
#else
    QBluetoothDeviceInfo info(QBluetoothAddress(addr_uuid), name
            , minor_dev_class << 2 | major_dev_class << 8 | serv_class << 13);
#endif

    ble = QLowEnergyController::createCentral(info, this);
    connect(ble, &QLowEnergyController::connected, this, &BlueTooth::ble_connectedSlot);
    connect(ble, &QLowEnergyController::connectionUpdated, this, &BlueTooth::ble_connectionUpdatedSlot);
    connect(ble, &QLowEnergyController::disconnected, this, &BlueTooth::ble_disconnectedSlot);
    connect(ble, &QLowEnergyController::discoveryFinished, this, &BlueTooth::ble_discoveryFinishedSlot);
    connect(ble, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(ble_errorSlot(QLowEnergyController::Error)));
    connect(ble, &QLowEnergyController::serviceDiscovered, this, &BlueTooth::ble_serviceDiscoveredSlot);
    connect(ble, &QLowEnergyController::stateChanged, this, &BlueTooth::ble_stateChangedSlot);

    ble_dev_name = name;
    ble_dev_addr_uuid = addr_uuid;
    ble->connectToDevice();
}

void BlueTooth::ble_connectedSlot()
{
    appendBleMessage(ble->remoteName() + tr(" 连接已建立"));
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_serv_view->model());
    int row = model->rowCount();
    for (int col = 0; col < model->columnCount(); col++)
    {
        QStandardItem* item = new QStandardItem();
        item->setEditable(false);

        switch (model->headerData(col, Qt::Horizontal, ble_serv_view_col_role).toInt())
        {
        case NameUUIDColRole:
            item->setData(ble_dev_name + " {" + ble_dev_addr_uuid + "}", Qt::DisplayRole);
            break;
        case TypeColRole:
            item->setData(tr("设备"), Qt::DisplayRole);
            item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
            break;
        default:
            // 即使没有显式设置 item(row, col)，在 view 视图下，它们所对应位置也可以被选中和编辑，
            // 然后自动创建 item，为避免该情况，需要特殊处理
            // item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setEnabled(false);
            break;
        }

        model->setItem(row, col, item);
        ui->ble_serv_view->expand(item->index());
    }

    ble->discoverServices();
}

void BlueTooth::ble_connectionUpdatedSlot(const QLowEnergyConnectionParameters &newParameters)
{
    (void)newParameters;
}

void BlueTooth::ble_disconnectedSlot()
{
    bleFree();
}

void BlueTooth::ble_discoveryFinishedSlot()
{
    if (! ble_serv_detail_timer)
    {
        ble_serv_detail_timer = new QTimer(this);
        connect(ble_serv_detail_timer, &QTimer::timeout, this, &BlueTooth::ble_serv_detail_timer_timeoutSlot);
    }
    ble_serv_detail_timer->start(500);
}

void BlueTooth::ble_serv_detail_timer_timeoutSlot()
{
    QLowEnergyService* serv = ble_serv_list.at(ble_serv_detail_index);
    /* 将当前服务的详情添加进视图 */
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_serv_view->model());
    // 新添加的设备默认都是最后一行
    QStandardItem* dev_item = model->item(model->rowCount() - 1);
    // 视图中若没有该服务，先添加服务
    bool none = true;
    int index = 0;
    for (; index < dev_item->rowCount(); index++)
    {
        if (dev_item->child(index)->data(ble_serv_uuid_role).toString()
                == serv->serviceUuid().toString())
        {
            none = false;
            break;
        }
    }
    if (none)
    {
        appendBleMessage(tr("发现一级服务：") + serv->serviceName() + " " + serv->serviceUuid().toString());
        int row = index;

        for (int col = 0; col < model->columnCount(); col++)
        {
            QStandardItem* item = new QStandardItem();
            item->setEditable(false);

            switch (model->headerData(col, Qt::Horizontal, ble_serv_view_col_role).toInt())
            {
            case NameUUIDColRole:
                item->setData(serv->serviceName() + " " + serv->serviceUuid().toString(), Qt::DisplayRole);
                item->setData(serv->serviceName(), ble_serv_name_role);
                item->setData(serv->serviceUuid().toString(), ble_serv_uuid_role);
                break;
            case TypeColRole:
                item->setData(tr("服务"), Qt::DisplayRole);
                item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
                break;
            default:
                item->setEnabled(false);
                break;
            }

            dev_item->setChild(row, col, item);
            ui->ble_serv_view->expand(item->index());
        }

        serv->discoverDetails();
    }
    // 视图中若没有该服务的特征值，则添加
    QList<QLowEnergyCharacteristic> chara_list = serv->characteristics();
    QStandardItem* serv_item = dev_item->child(index);
    for (const QLowEnergyCharacteristic& chara: qAsConst(chara_list))
    {
        none = true;
        for (int i = 0; i < serv_item->rowCount(); i++)
        {
            if (chara.uuid().toString() == serv_item->child(i)->data(ble_chara_uuid_role).toString())
            {
                none = false;
                break;
            }
        }
        if (none)
        {
            appendBleMessage(tr("发现特征值：") + chara.name() + " " + chara.uuid().toString());
            int row = serv_item->rowCount();

            for (int col = 0; col < model->columnCount(); col++)
            {
                QStandardItem* item = new QStandardItem();
                item->setEditable(false);

                switch (model->headerData(col, Qt::Horizontal, ble_serv_view_col_role).toInt())
                {
                case NameUUIDColRole:
                    item->setData(chara.name() + " " + chara.uuid().toString(), Qt::DisplayRole);
                    item->setData(chara.name(), ble_chara_name_role);
                    item->setData(chara.uuid().toString(), ble_chara_uuid_role);
                    break;
                case TypeColRole:
                    item->setData(tr("特征值"), Qt::DisplayRole);
                    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
                    break;
                case PermissionColRole:
                    item->setData(bleCharaPerm2Str(chara.properties()), Qt::DisplayRole);
                    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
                    break;
                case ValueColRole:
                    item->setData(QString(chara.value().toHex().toUpper()), Qt::DisplayRole);
                    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
                    break;
                default:
                    item->setEnabled(false);
                    break;
                }

                // 添加特征值包含的描述符
                QList<QLowEnergyDescriptor> desc_list = chara.descriptors();
                for (int i = 0; i < desc_list.count(); i++)
                {
                    QLowEnergyDescriptor desc = desc_list.at(i);
                    for (int j = 0; j < model->columnCount(); j++)
                    {
                        QStandardItem* item_child = new QStandardItem();
                        item_child->setEditable(false);

                        switch (model->headerData(j, Qt::Horizontal, ble_serv_view_col_role).toInt())
                        {
                        case NameUUIDColRole:
                            item_child->setData(desc.name() + " " + desc.uuid().toString(), Qt::DisplayRole);
                            item_child->setData(desc.name(), ble_desc_name_role);
                            item_child->setData(desc.uuid().toString(), ble_desc_uuid_role);
                            break;
                        case TypeColRole:
                            item_child->setData(tr("描述"), Qt::DisplayRole);
                            item_child->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
                            break;
                        case ValueColRole: // 不要看这个语句啥也没干，实际上它跳过了后面的 default
                            break;
                        default:
                            item_child->setEnabled(false);
                            break;
                        }
                        item->setChild(i, j, item_child);
                    }
                }

                serv_item->setChild(row, col, item);
                ui->ble_serv_view->expand(item->index());
            }
        }
    }

    switch (serv->state())
    {
    case QLowEnergyService::ServiceDiscovered:
        ble_serv_detail_index++;
        // 若所有服务详情获取完毕，则停止定时器
        if (ble_serv_detail_index >= ble_serv_list.count())
        {
            ble_serv_detail_timer->stop();
            appendBleMessage(tr("已搜索全部服务"));
        }
        break;
    default:
        break;
    }
}

void BlueTooth::ble_errorSlot(QLowEnergyController::Error newError)
{
    appendBleMessage(tr("ble 错误：%1").arg(newError));
}

void BlueTooth::ble_serviceDiscoveredSlot(const QBluetoothUuid &newService)
{
    QLowEnergyService* serv = ble->createServiceObject(newService, ble);
    ble_serv_list.append(serv);
}

void BlueTooth::ble_stateChangedSlot(QLowEnergyController::ControllerState state)
{
    (void)state;
}

void BlueTooth::appendBleMessage(QString msg)
{
    ui->ble_msg_area->append(
            QDateTime::currentDateTime().toString("[yyyy.MM.dd HH:mm:ss:zzz] ") + msg);
}

void BlueTooth::on_ble_disconnect_btn_clicked()
{
    if (ble)
    {
        if (ble->state() == QLowEnergyController::UnconnectedState)
            bleFree();
        else
            ble->disconnectFromDevice();
    }
    else
        appendBleMessage(tr("无可用 ble 连接"));
}

void BlueTooth::bleFree()
{
    appendBleMessage(ble->remoteName() + tr(" 释放连接资源"));
    // 释放连接资源
    if (ble_serv_detail_timer)
    {
        ble_serv_detail_timer->stop();
        delete ble_serv_detail_timer;
        // ble_serv_detail_timer->deleteLater();
        ble_serv_detail_timer = Q_NULLPTR;
    }
    if (ble)
    {
        delete ble;
        // ble->deleteLater();
        ble = Q_NULLPTR;
    }
    ble_serv_list.clear();
    ble_serv_detail_index = 0;
    ble_dev_name.clear();
    ble_dev_addr_uuid.clear();
    // 清空服务视图
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_serv_view->model());
    model->clear();
    bleServViewHorHeadInit();
}

void BlueTooth::bleServViewHorHeadInit()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_serv_view->model());
    model->setColumnCount(4);
    // 首列项作为主项，描述设备、服务、特征值等，不移动
    model->setHeaderData(0, Qt::Horizontal, tr("名称 {UUID}"), Qt::DisplayRole);
    model->setHeaderData(0, Qt::Horizontal, NameUUIDColRole, ble_serv_view_col_role);
    model->setHeaderData(1, Qt::Horizontal, tr("类型"), Qt::DisplayRole);
    model->setHeaderData(1, Qt::Horizontal, TypeColRole, ble_serv_view_col_role);
    model->setHeaderData(2, Qt::Horizontal, tr("权限"), Qt::DisplayRole);
    model->setHeaderData(2, Qt::Horizontal, PermissionColRole, ble_serv_view_col_role);
    model->setHeaderData(3, Qt::Horizontal, tr("值"), Qt::DisplayRole);
    model->setHeaderData(3, Qt::Horizontal, ValueColRole, ble_serv_view_col_role);

    ui->ble_serv_view->header()->setDefaultAlignment(Qt::AlignCenter);
    QHeaderView* header = ui->ble_serv_view->header();
    header->resizeSection(0, 500);
    header->resizeSection(1, 100);
    header->resizeSection(2, 100);
    header->show();
}

QString BlueTooth::bleCharaPerm2Str(QLowEnergyCharacteristic::PropertyTypes p)
{
    QString str("|");
    if (QLowEnergyCharacteristic::Unknown == p)
        return tr("|未知|");
    if (p & QLowEnergyCharacteristic::Broadcasting)
        str += tr("广播|");
    if (p & QLowEnergyCharacteristic::Read)
        str += tr("读|");
    if (p & QLowEnergyCharacteristic::WriteNoResponse)
        str += tr("写无回应|");
    if (p & QLowEnergyCharacteristic::Write)
        str += tr("写|");
    if (p & QLowEnergyCharacteristic::Notify)
        str += tr("通知|");
    if (p & QLowEnergyCharacteristic::Indicate)
        str += tr("指示|");
    if (p & QLowEnergyCharacteristic::WriteSigned)
        str += tr("写签名|");
    if (p & QLowEnergyCharacteristic::ExtendedProperty)
        str += tr("扩展|");
    return str;
}

void BlueTooth::on_ble_read_btn_clicked()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->ble_serv_view->model());
    QStandardItem* item = model->itemFromIndex(ui->ble_serv_view->currentIndex());
    if (Q_NULLPTR == item)
    {
        appendBleMessage(tr("试图读取无效项"));
        return;
    }
    int cur_row = item->row();
    QStandardItem* item_parent = item->parent();
    if (Q_NULLPTR == item_parent)
    {
        appendBleMessage(tr("不允许读取根项"));
        return;
    }
    int name_uuid_col = 0;
    for (; name_uuid_col < model->columnCount(); name_uuid_col++)
    {
        if (model->headerData(name_uuid_col, Qt::Horizontal, ble_serv_view_col_role) == NameUUIDColRole)
            break;
    }
    item = item_parent->child(cur_row, name_uuid_col);
    if (item->data(ble_chara_uuid_role).isValid())
    {
        // TODO
    }
    else if (item->data(ble_desc_uuid_role).isValid())
    {
        // TODO
    }
    else
    {
        // TODO
    }
}
