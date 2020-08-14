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

// Qt DBus
#include <QDBusConnection>
#include <QDBusInterface>

#include <KWindowSystem>

// Function to create a new Fm::FileDialogHelper object.
// This is dynamically loaded at runtime on demand from libfm-qt.
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

PandaPlatformTheme::PandaPlatformTheme()
{
    m_hints = new HintsSettings();

    qApp->setProperty("_hints_settings_object", (quintptr)m_hints);

    if (KWindowSystem::isPlatformX11()) {
        m_x11Integration.reset(new X11Integration());
        m_x11Integration->init();
    }

    // auto onFontChanged = [=] {
    //                          QEvent event(QEvent::ApplicationFontChange);
    //                          qApp->sendEvent(qApp, &event);

    //                          for (QWindow *window : qGuiApp->allWindows()) {
    //                              if (window->type() == Qt::Desktop)
    //                                  continue;


    //                              qApp->sendEvent(window, &event);
    //                          }

    //                          Q_EMIT qGuiApp->fontChanged(qGuiApp->font());
    //                      };

    connect(m_hints, &HintsSettings::darkModeChanged, this, [=] {
        QApplication::setStyle("panda");
    });

    connect(m_hints, &HintsSettings::systemFontChanged, this, [=] {
        QString fontFamily = m_hints->systemFont();
        QFont font = QApplication::font();
        font.setFamily(fontFamily);
        QApplication::setFont(fontFamily);
    });

    connect(m_hints, &HintsSettings::systemFontPointSizeChanged, this, [=] {
        QFont font = QApplication::font();
        font.setPointSizeF(m_hints->systemFontPointSize());
        font.setFamily(m_hints->systemFont());
        QApplication::setFont(font);
    });

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
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
    if (type == FileDialog
       && qobject_cast<QApplication *>(QCoreApplication::instance())) { // QML may not have qApp
        // use our own file dialog provided by libfm

        // When a process has this environment set, that means glib event loop integration is disabled.
        // In this case, libfm-qt just won't work. So let's disable the file dialog helper and return nullptr.
        if (QString::fromLocal8Bit(qgetenv("QT_NO_GLIB")) == QLatin1String("1")) {
            return nullptr;
        }

        // The createFileDialogHelper() method is dynamically loaded from libfm-qt on demand
        if (createFileDialogHelper == nullptr) {
            // try to dynamically load versioned libfm-qt.so
            QLibrary fmLibrary{QLatin1String(LIB_FM_QT_SONAME)};
            fmLibrary.load();
            if (!fmLibrary.isLoaded()) {
                return nullptr;
            }

            // try to resolve the symbol to get the function pointer
            createFileDialogHelper = reinterpret_cast<CreateFileDialogHelperFunc>(fmLibrary.resolve("createFileDialogHelper"));
            if (!createFileDialogHelper) {
                return nullptr;
            }
        }

        // create a new file dialog helper provided by libfm
        return createFileDialogHelper();
    }
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
