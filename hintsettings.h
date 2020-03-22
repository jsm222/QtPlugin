#ifndef HINTSSETTINGS_H
#define HINTSSETTINGS_H

#include <QDBusVariant>
#include <QObject>
#include <QVariant>

#include <qpa/qplatformtheme.h>

class QPalette;
class HintsSettings : public QObject
{
    Q_OBJECT
    
public:
    explicit HintsSettings(QObject *parent = nullptr);
    ~HintsSettings() override;

    QStringList xdgIconThemePaths() const;

    inline QVariant hint(QPlatformTheme::ThemeHint hint) const {
        return m_hints[hint];
    }
    
private:
    QHash<QPlatformTheme::ThemeHint, QVariant> m_hints;
};

#endif //HINTSSETTINGS_H
