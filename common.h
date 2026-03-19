#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QFile>
#include <QTextStream>

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);

    static QString Style(QString filename);

};

#endif // COMMON_H
