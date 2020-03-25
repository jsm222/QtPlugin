#ifndef PSTYLEPLUGIN_H
#define PSTYLEPLUGIN_H

#include <QStylePlugin>

class ProxyStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "pstyleplugin.json")

public:
    ProxyStylePlugin();

    QStyle *create(const QString &key) override;
};

#endif // PSTYLEPLUGIN_H