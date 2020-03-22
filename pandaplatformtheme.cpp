#include "pandaplatformtheme.h"
#include "x11integration.h"
#include "qdbusmenubar_p.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QVariant>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

#include <KWindowSystem>

static const QByteArray s_x11AppMenuServiceNamePropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_SERVICE_NAME");
static const QByteArray s_x11AppMenuObjectPathPropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_OBJECT_PATH");

static bool checkDBusGlobalMenuAvailable()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString registrarService = QStringLiteral("com.canonical.AppMenu.Registrar");
    return connection.interface()->isServiceRegistered(registrarService);
}

static bool isDBusGlobalMenuAvailable()
{
    static bool dbusGlobalMenuAvailable = checkDBusGlobalMenuAvailable();
    return dbusGlobalMenuAvailable;
}

PandaPlatformTheme::PandaPlatformTheme()
{
    m_hints = new HintsSettings;
    
    if (KWindowSystem::isPlatformX11()) {
        m_x11Integration.reset(new X11Integration());
        m_x11Integration->init();
    }

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
}

PandaPlatformTheme::~PandaPlatformTheme()
{
}

QVariant PandaPlatformTheme::themeHint(QPlatformTheme::ThemeHint hintType) const
{
    QVariant hint = m_hints->hint(hintType);
    if (hint.isValid()) {
        return hint;
    } else {
        return QPlatformTheme::themeHint(hintType);
    }
}

QPlatformMenuBar *PandaPlatformTheme::createPlatformMenuBar() const
{
    if (isDBusGlobalMenuAvailable()) {
        auto *menu = new QDBusMenuBar();

        QObject::connect(menu, &QDBusMenuBar::windowChanged, menu, [this, menu](QWindow *newWindow, QWindow *oldWindow) {
            const QString &serviceName = QDBusConnection::sessionBus().baseService();
            const QString &objectPath = menu->objectPath();

            if (m_x11Integration) {
                if (oldWindow) {
                    m_x11Integration->setWindowProperty(oldWindow, s_x11AppMenuServiceNamePropertyName, {});
                    m_x11Integration->setWindowProperty(oldWindow, s_x11AppMenuObjectPathPropertyName, {});
                }

                if (newWindow) {
                    m_x11Integration->setWindowProperty(newWindow, s_x11AppMenuServiceNamePropertyName, serviceName.toUtf8());
                    m_x11Integration->setWindowProperty(newWindow, s_x11AppMenuObjectPathPropertyName, objectPath.toUtf8());
                }
            }

//             if (m_kwaylandIntegration) {
//                 if (oldWindow) {
//                     m_kwaylandIntegration->setAppMenu(oldWindow, QString(), QString());
//                 }
// 
//                 if (newWindow) {
//                     m_kwaylandIntegration->setAppMenu(newWindow, serviceName, objectPath);
//                 }
//             }
        });

        return menu;
    }

    return nullptr;
}
