#include <qpa/qplatformthemeplugin.h>
#include "pandaplatformtheme.h"

class KdePlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "panda.json")
public:
    KdePlatformThemePlugin(QObject *parent = nullptr)
        : QPlatformThemePlugin(parent) {}

    QPlatformTheme *create(const QString &key, const QStringList &paramList) override
    {
        Q_UNUSED(key)
        Q_UNUSED(paramList)
        return new PandaPlatformTheme;
    }
};

#include "main.moc"
