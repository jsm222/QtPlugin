#include "pstyleplugin.h"
#include "style.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

ProxyStylePlugin::ProxyStylePlugin()
{
    qDebug() << "init panda style plugin";
}

QStyle *ProxyStylePlugin::create(const QString &key)
{
    return new Style;
}