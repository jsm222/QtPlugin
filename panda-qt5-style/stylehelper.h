#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <qglobal.h>
#include <qpa/qplatformtheme.h>
#include <QCoreApplication>
#include <QStyleOption>
#include <QPixmapCache>
#include <QPainter>
#include <QImage>

#include <private/qfixed_p.h>
#include <private/qtextengine_p.h>
#include <private/qicon_p.h>

Q_GUI_EXPORT int qt_defaultDpiX();
Q_GUI_EXPORT int qt_defaultDpiY();

QT_BEGIN_NAMESPACE
//extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

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
    //    return pal.color(QPalette::Highlight);

        return pal.highlight().color();
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

    // ref: https://github.com/linuxdeepin/dtkwidget/blob/master/src/widgets/dstyle.cpp
    static QList<QRect> sudokuByRect(const QRect &rect, QMargins borders)
    {
        QList<QRect> list;

    //    qreal border_width = borders.left() + borders.right();

    //    if ( border_width > rect.width()) {
    //        borders.setLeft(borders.left() / border_width * rect.width());
    //        borders.setRight(rect.width() - borders.left());
    //    }

    //    qreal border_height = borders.top() + borders.bottom();

    //    if (border_height > rect.height()) {
    //        borders.setTop(borders.top()/ border_height * rect.height());
    //        borders.setBottom(rect.height() - borders.top());
    //    }

        const QRect &contentsRect = rect - borders;

        list << QRect(0, 0, borders.left(), borders.top());
        list << QRect(list.at(0).topRight(), QSize(contentsRect.width(), borders.top())).translated(1, 0);
        list << QRect(list.at(1).topRight(), QSize(borders.right(), borders.top())).translated(1, 0);
        list << QRect(list.at(0).bottomLeft(), QSize(borders.left(), contentsRect.height())).translated(0, 1);
        list << contentsRect;
        list << QRect(contentsRect.topRight(), QSize(borders.right(), contentsRect.height())).translated(1, 0);
        list << QRect(list.at(3).bottomLeft(), QSize(borders.left(), borders.bottom())).translated(0, 1);
        list << QRect(contentsRect.bottomLeft(), QSize(contentsRect.width(), borders.bottom())).translated(0, 1);
        list << QRect(contentsRect.bottomRight(), QSize(borders.left(), borders.bottom())).translated(1, 1);

        return list;
    }

    static QImage borderImage(const QPixmap &px, const QMargins &borders, const QSize &size, QImage::Format format)
    {
        QImage image(size, format);
        QPainter pa(&image);

        const QList<QRect> sudoku_src = sudokuByRect(px.rect(), borders);
        const QList<QRect> sudoku_tar = sudokuByRect(QRect(QPoint(0, 0), size), borders);

        pa.setCompositionMode(QPainter::CompositionMode_Source);  //设置组合模式

        for (int i = 0; i < 9; ++i) {
            pa.drawPixmap(sudoku_tar[i], px, sudoku_src[i]);
        }

        pa.end();

        return image;
    }

    static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color)
    {
        if (px.isNull())
            return QImage();

        QImage tmp(px.size() + QSize(radius * 2, radius * 2), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(0);
        QPainter tmpPainter(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
        tmpPainter.drawPixmap(QPoint(radius, radius), px);
        tmpPainter.end();

        // blur the alpha channel
        QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
        blurred.fill(0);
        QPainter blurPainter(&blurred);
        qt_blurImage(&blurPainter, tmp, radius, false, true);
        blurPainter.end();

        if (color == QColor(Qt::black))
            return blurred;

        tmp = blurred;

        // blacken the image...
        tmpPainter.begin(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        return tmp;
    }

    static void drawShadow(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, const QColor &sc, qreal radius, const QPoint &offset)
    {
        QPixmap shadow;
        qreal scale = pa->paintEngine()->paintDevice()->devicePixelRatioF();
        QRect shadow_rect = rect;

        shadow_rect.setTopLeft(shadow_rect.topLeft() + offset);

        xRadius *= scale;
        yRadius *= scale;
        radius *= scale;

        const QString &key = QString("dtk-shadow-%1x%2-%3-%4").arg(xRadius).arg(yRadius).arg(sc.name()).arg(radius);

        if (!QPixmapCache::find(key, shadow)) {
            QImage shadow_base(QSize(xRadius * 3, yRadius * 3), QImage::Format_ARGB32_Premultiplied);
            shadow_base.fill(0);
            QPainter pa(&shadow_base);

            pa.setBrush(sc);
            pa.setPen(Qt::NoPen);
            pa.drawRoundedRect(shadow_base.rect(), xRadius, yRadius);
            pa.end();

            shadow_base = dropShadow(QPixmap::fromImage(shadow_base), radius, sc);
            shadow = QPixmap::fromImage(shadow_base);
            QPixmapCache::insert(key, shadow);
        }

        const QMargins margins(xRadius + radius, yRadius + radius, xRadius + radius, yRadius + radius);
        QImage new_shadow = borderImage(shadow, margins, shadow_rect.size() * scale, QImage::Format_ARGB32_Premultiplied);
    //    QPainter pa_shadow(&new_shadow);
    //    pa_shadow.setCompositionMode(QPainter::CompositionMode_Clear);
    //    pa_shadow.setPen(Qt::NoPen);
    //    pa_shadow.setBrush(Qt::transparent);
    //    pa_shadow.setRenderHint(QPainter::Antialiasing);
    //    pa_shadow.drawRoundedRect((new_shadow.rect() - QMargins(radius, radius, radius, radius)).translated(-offset), xRadius, yRadius);
    //    pa_shadow.end();
        new_shadow.setDevicePixelRatio(scale);
        pa->drawImage(shadow_rect.topLeft(), new_shadow);
    }

    enum {
        menuItemHMargin      =  3, // menu item hor text margin
        menuArrowHMargin     =  6, // menu arrow horizontal margin
        menuRightBorder      = 15, // right border on menus
        menuCheckMarkWidth   = 12  // checkmarks width on menus
    };
};

#endif //STYLEHELPER_H
