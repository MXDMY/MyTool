#include "common.h"

Common::Common(QObject *parent) : QObject(parent)
{

}

QString Common::Style(QString filename)
{
    QFile file(":/qss/" + filename);
    QString qss;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        stream.setCodec("UTF-8"); // 根据实际编码设置
        qss = stream.readAll();
        file.close();
    }

    return qss;
}
