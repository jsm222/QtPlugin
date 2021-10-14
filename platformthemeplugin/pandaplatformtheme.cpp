#include "pandaplatformtheme.h"
#include "x11integration.h"
#include "qdbusmenubar_p.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QLibrary>
#include <QStyleFactory>

// Qt Private
#include <private/qicon_p.h>
#include <private/qiconloader_p.h>
#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>

// Qt DBus
#include <QDBusConnection>
#include <QDBusInterface>

#include <KWindowSystem>

// Function to create a new Fm::FileDialogHelper object.
// This is dynamically loaded at runtime on demand from pandafm.
typedef QPlatformDialogHelper* (*CreateFileDialogHelperFunc)();
static CreateFileDialogHelperFunc createFileDialogHelper = nullptr;

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

static void onFontChanged()
{
    if (QGuiApplicationPrivate::app_font)
        delete QGuiApplicationPrivate::app_font;

    QGuiApplicationPrivate::app_font = nullptr;
    QEvent event(QEvent::ApplicationFontChange);
    qApp->sendEvent(qApp, &event);

    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &event);
    }

    Q_EMIT qGuiApp->fontChanged(qGuiApp->font());
}

extern void updateXdgIconSystemTheme();
static void onIconThemeChanged()
{
    QIconLoader::instance()->updateSystemTheme();
    updateXdgIconSystemTheme();

    QEvent update(QEvent::UpdateRequest);
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &update);
    }
}

void onDarkModeChanged()
{
    QStyle *style = QStyleFactory::create("panda");
    if (style) {
        qApp->setStyle(style);
    }
}

PandaPlatformTheme::PandaPlatformTheme()
    : m_hints(new HintsSettings)
{
    // qApp->setProperty("_hints_settings_object", (quintptr)m_hints);

    if (KWindowSystem::isPlatformX11()) {
        m_x11Integration.reset(new X11Integration());
        m_x11Integration->init();
    }

    connect(m_hints, &HintsSettings::systemFontChanged, &onFontChanged);
    connect(m_hints, &HintsSettings::systemFontPointSizeChanged, &onFontChanged);
    connect(m_hints, &HintsSettings::iconThemeChanged, &onIconThemeChanged);
    connect(m_hints, &HintsSettings::darkModeChanged, &onDarkModeChanged);

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true); // probono: need to use myAction->setIconVisibleInMenu(true); for menu items that shall get the icon nevertheless
}

PandaPlatformTheme::~PandaPlatformTheme()
{
}

bool PandaPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    if (type == FileDialog
       && qobject_cast<QApplication *>(QCoreApplication::instance())) { // QML may not have qApp
        // use our own file dialog
        return true;
    }
    return false;
}

QPlatformDialogHelper *PandaPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    return nullptr;
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

const QFont* PandaPlatformTheme::font(Font type) const
{
    switch (type) {
    case SystemFont: {
        const QString &fontName = m_hints->systemFont();
        qreal fontSize = m_hints->systemFontPointSize();
        static QFont font = QFont(QString());
        font.setFamily(fontName);
        font.setPointSizeF(fontSize);
        return &font;
    }
    case FixedFont: {
        const QString &fontName = m_hints->systemFixedFont();
        qreal fontSize = m_hints->systemFontPointSize();
        static QFont font = QFont(QString());
        font.setFamily(fontName);
        font.setPointSizeF(fontSize);
        return &font;
    }
    default:
        break;
    }

    return QPlatformTheme::font(type);
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
