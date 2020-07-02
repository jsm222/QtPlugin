#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <qglobal.h>
#include <qpa/qplatformtheme.h>
#include <QCoreApplication>
#include <QStyleOption>

Q_GUI_EXPORT int qt_defaultDpiX();
Q_GUI_EXPORT int qt_defaultDpiY();

static const qreal qstyleBaseDpi = 96;

class StyleHelper
{
public:
    StyleHelper() {}

    // Used for grip handles
    static QColor lightShade() {
        return QColor(255, 255, 255, 90);
    }
    static QColor darkShade() {
        return QColor(0, 0, 0, 60);
    }

    static QColor topShadow() {
        return QColor(0, 0, 0, 18);
    }

    static QColor innerContrastLine() {
        return QColor(255, 255, 255, 30);
    }

    static QColor highlight(const QPalette &pal) {
    //    if (isMacSystemPalette(pal))
    //        return QColor(60, 140, 230);
        return pal.color(QPalette::Highlight);
    }

    static QColor highlightedText(const QPalette &pal) {
        // if (isMacSystemPalette(pal))
        //     return Qt::white;
        return pal.color(QPalette::HighlightedText);
    }

    static QColor outline(const QPalette &pal) {
        if (pal.window().style() == Qt::TexturePattern)
            return QColor(0, 0, 0, 160);
        return pal.window().color().darker(140);
    }

    static QColor highlightedOutline(const QPalette &pal) {
        QColor highlightedOutline = highlight(pal).darker(125);
        if (highlightedOutline.value() > 160)
            highlightedOutline.setHsl(highlightedOutline.hue(), highlightedOutline.saturation(), 160);
        return highlightedOutline;
    }

    static QColor tabFrameColor(const QPalette &pal) {
        if (pal.window().style() == Qt::TexturePattern)
            return QColor(255, 255, 255, 8);
        return buttonColor(pal).lighter(104);
    }

    static QColor buttonColor(const QPalette &pal) {
        QColor buttonColor = pal.button().color();
        int val = qGray(buttonColor.rgb());
        buttonColor = buttonColor.lighter(100 + qMax(1, (180 - val)/6));
        buttonColor.setHsv(buttonColor.hue(), buttonColor.saturation() * 0.75, buttonColor.value());
        return buttonColor;
    }

    // DPI
    static qreal dpi(const QStyleOption *option)
    {
    #ifndef Q_OS_DARWIN
        // Prioritize the application override, except for on macOS where
        // we have historically not supported the AA_Use96Dpi flag.
        if (QCoreApplication::testAttribute(Qt::AA_Use96Dpi))
            return 96;
    #endif

        // Expect that QStyleOption::QFontMetrics::QFont has the correct DPI set
        if (option)
            return option->fontMetrics.fontDpi();

        // Fall back to historical Qt behavior: hardocded 72 DPI on mac,
        // primary screen DPI on other platforms.
    #ifdef Q_OS_DARWIN
        return qstyleBaseDpi;
    #else
        return qt_defaultDpiX();
    #endif
    }

    static qreal dpiScaled(qreal value, qreal dpi)
    {
        return value * dpi / qstyleBaseDpi;
    }

    static qreal dpiScaled(qreal value, const QPaintDevice *device)
    {
        return dpiScaled(value, device->logicalDpiX());
    }

    static qreal dpiScaled(qreal value, const QStyleOption *option)
    {
        return dpiScaled(value, dpi(option));
    }
    // DPI end

    enum {
        menuItemHMargin      =  3, // menu item hor text margin
        menuArrowHMargin     =  6, // menu arrow horizontal margin
        menuRightBorder      = 15, // right border on menus
        menuCheckMarkWidth   = 12  // checkmarks width on menus
    };
};

#endif //STYLEHELPER_H
