#include "modernstyle.h"
#include "stylehelper.h"
#include <QPainter>
#include <QStyleOption>
#include <QMainWindow>
#include <QComboBox>
#include <QSplitterHandle>
#include <QScrollBar>
#include <QProgressBar>
#include <QAbstractButton>
#include <QPainterPath>
#include <QApplication>
#include <QMenu>

enum Direction {
    TopDown,
    FromLeft,
    BottomUp,
    FromRight
};

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

static const QByteArray s_blurBehindPropertyName = QByteArrayLiteral("ENABLE_BLUR_BEHIND_HINT");

// from windows style
static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  8; // menu item ver text margin
static const int windowsRightBorder      = 15; // right border on windows

static const int groupBoxBottomMargin    =  0;  // space below the groupbox
static const int groupBoxTopMargin       =  3;

static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : 0;
}

static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

// The default button and handle gradient
static QLinearGradient qt_fusion_gradient(const QRect &rect, const QBrush &baseColor, Direction direction = TopDown)
{
    int x = rect.center().x();
    int y = rect.center().y();
    QLinearGradient gradient;
    switch (direction) {
    case FromLeft:
        gradient = QLinearGradient(rect.left(), y, rect.right(), y);
        break;
    case FromRight:
        gradient = QLinearGradient(rect.right(), y, rect.left(), y);
        break;
    case BottomUp:
        gradient = QLinearGradient(x, rect.bottom(), x, rect.top());
        break;
    case TopDown:
    default:
        gradient = QLinearGradient(x, rect.top(), x, rect.bottom());
        break;
    }
    if (baseColor.gradient())
        gradient.setStops(baseColor.gradient()->stops());
    else {
        QColor gradientStartColor = baseColor.color().lighter(124);
        QColor gradientStopColor = baseColor.color().lighter(102);
        gradient.setColorAt(0, gradientStartColor);
        gradient.setColorAt(1, gradientStopColor);
        //          Uncomment for adding shiny shading
        //            QColor midColor1 = mergedColors(gradientStartColor, gradientStopColor, 55);
        //            QColor midColor2 = mergedColors(gradientStartColor, gradientStopColor, 45);
        //            gradient.setColorAt(0.5, midColor1);
        //            gradient.setColorAt(0.501, midColor2);
    }
    return gradient;
}

ModernStyle::ModernStyle()
  : m_blurHelper(new BlurHelper(this))
{
    setObjectName(QLatin1String("Modern"));
}

ModernStyle::~ModernStyle()
{
}

QPalette ModernStyle::standardPalette() const
{
    QColor backGround(255, 255, 255);
    QColor light = backGround.lighter(150);
    QColor mid(backGround.darker(130));
    QColor midLight = mid.lighter(110);
    QColor base = Qt::white;
    QColor disabledBase(backGround);
    QColor dark = backGround.darker(150);
    QColor darkDisabled = QColor(209, 209, 209).darker(110);
    QColor text = Qt::black;
    QColor hightlightedText = Qt::white;
    QColor disabledText = QColor(190, 190, 190);
    QColor button = backGround;
    QColor shadow = dark.darker(135);
    QColor disabledShadow = shadow.lighter(150);
    QColor placeholder = text;
    QColor highlightColor(84, 156, 255);
    placeholder.setAlpha(128);

    QPalette fusionPalette(Qt::black, backGround, light, dark, mid, text, base);
    fusionPalette.setBrush(QPalette::Midlight, midLight);
    fusionPalette.setBrush(QPalette::Button, button);
    fusionPalette.setBrush(QPalette::Shadow, shadow);
    fusionPalette.setBrush(QPalette::HighlightedText, hightlightedText);

    fusionPalette.setBrush(QPalette::Disabled, QPalette::Text, disabledText);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::WindowText, disabledText);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledText);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::Base, disabledBase);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::Dark, darkDisabled);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::Shadow, disabledShadow);

    fusionPalette.setBrush(QPalette::Highlight, highlightColor);
    fusionPalette.setBrush(QPalette::Active, QPalette::Highlight, highlightColor);
    fusionPalette.setBrush(QPalette::Inactive, QPalette::Highlight, highlightColor);
    fusionPalette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 145, 145));

    fusionPalette.setBrush(QPalette::Base, base);
    fusionPalette.setBrush(QPalette::Window, base);

    fusionPalette.setBrush(QPalette::PlaceholderText, placeholder);

    return fusionPalette;
}

void ModernStyle::drawPrimitive(PrimitiveElement elem,
                                 const QStyleOption *option,
                                 QPainter *painter, const QWidget *widget) const
{
    Q_ASSERT(option);

    QRect rect = option->rect;
    int state = option->state;

    QColor outline = StyleHelper::outline(option->palette);
    QColor highlightedOutline = StyleHelper::highlightedOutline(option->palette);
    QColor tabFrameColor = StyleHelper::tabFrameColor(option->palette);
    QColor topShadow = QColor(0, 0, 0, 18);

    switch (elem) {
    case PE_IndicatorBranch: {
        if (!(option->state & State_Children))
            break;
        if (option->state & State_Open)
            drawPrimitive(PE_IndicatorArrowDown, option, painter, widget);
        else {
            const bool reverse = (option->direction == Qt::RightToLeft);
            drawPrimitive(reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight, option, painter, widget);
        }
        break;
    }
    case PE_IndicatorHeaderArrow: {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(Qt::NoBrush);
            if(option->state & State_Enabled){
                painter->setPen(QPen(option->palette.windowText().color(), 1.1));
                if (option->state & State_MouseOver) {
                    painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
                }
            }
            else {
                painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
            }
            QPolygon points(5);
            //Add 8 to center vertically
            int x = option->rect.x() + 9;
            int y = option->rect.y() + 9;

            int w = 8;
            int h =  4;
            x += (option->rect.width() - w) / 2;
            y += (option->rect.height() - h) / 2;
            if (header->sortIndicator & QStyleOptionHeader::SortUp) {
                points[0] = QPoint(x, y);
                points[1] = QPoint(x + w / 2, y + h);
                points[2] = QPoint(x + w / 2, y + h);
                points[3] = QPoint(x + w, y);
            } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
                points[0] = QPoint(x, y + h);
                points[1] = QPoint(x + w / 2, y);
                points[2] = QPoint(x + w / 2, y);
                points[3] = QPoint(x + w, y + h);
            }
            painter->drawLine(points[0],  points[1] );
            painter->drawLine(points[2],  points[3] );
            painter->restore();
        }
    }
        break;
    // tabbar
    case PE_FrameTabBarBase:
        break;
    case PE_Frame:
        break;
    case PE_PanelMenu:
    case PE_FrameMenu:
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, QPainter::SmoothPixmapTransform);
    {
        int radius = frameRadius;
        QPainterPath rectPath;
        rectPath.addRoundedRect(option->rect.adjusted(radius, radius, -radius, -radius), radius, radius);

        QPixmap pixmap(option->rect.size());
        pixmap.fill(Qt::transparent);
        QPainter pixmapPainter(&pixmap);
        pixmapPainter.setRenderHint(QPainter::Antialiasing);
        pixmapPainter.setPen(Qt::transparent);
        pixmapPainter.setBrush(Qt::black);
        pixmapPainter.drawPath(rectPath);
        pixmapPainter.end();

        QImage img = pixmap.toImage();
        qt_blurImage(img, 5, false, false);

        pixmap = QPixmap::fromImage(img);
        QPainter pixmapPainter2(&pixmap);
        pixmapPainter2.setRenderHint(QPainter::Antialiasing);
        pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
        pixmapPainter2.setPen(Qt::transparent);
        pixmapPainter2.setBrush(Qt::transparent);
        pixmapPainter2.drawPath(rectPath);
        painter->drawPixmap(option->rect, pixmap, pixmap.rect());

        QColor color = option->palette.color(QPalette::Base);
        color.setAlpha(200);
        painter->setPen(Qt::transparent);
        painter->setBrush(color);

        QPainterPath path;
        QRegion region = widget->mask();
        if (region.isEmpty()) {
            path.addRoundedRect(option->rect.adjusted(radius, radius, -radius, -radius), radius, radius);
        } else {
            path.addRegion(region);
        }

        painter->drawPath(path);
    }
        painter->restore();
        break;
    case PE_FrameDockWidget:
        painter->save();
    {
        QColor softshadow = option->palette.window().color().darker(120);

        QRect rect= option->rect;
        painter->setPen(softshadow);
        painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
        painter->setPen(QPen(option->palette.light(), 1));
        painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1), QPoint(rect.left() + 1, rect.bottom() - 1));
        painter->setPen(QPen(option->palette.window().color().darker(120)));
        painter->drawLine(QPoint(rect.left() + 1, rect.bottom() - 1), QPoint(rect.right() - 2, rect.bottom() - 1));
        painter->drawLine(QPoint(rect.right() - 1, rect.top() + 1), QPoint(rect.right() - 1, rect.bottom() - 1));
    }
        painter->restore();
        break;
    case PE_PanelButtonTool:
        painter->save();
        if ((option->state & State_Enabled || option->state & State_On) || !(option->state & State_AutoRaise)) {
            if (widget && widget->inherits("QDockWidgetTitleButton")) {
                if (option->state & State_MouseOver)
                    proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            } else {
                proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            }
        }
        painter->restore();
        break;
    case PE_PanelItemViewItem: {
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QPalette::ColorGroup cg = (widget ? widget->isEnabled() : (vopt->state & QStyle::State_Enabled))
                ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                cg = QPalette::Inactive;
            const bool isIconView = (vopt->decorationPosition & QStyleOptionViewItem::Top);

            if (vopt->state & QStyle::State_Selected) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(Qt::transparent);
                painter->setBrush(vopt->palette.brush(cg, QPalette::Highlight));

                if (isIconView) {
                    double radius = option->rect.height() * m_radiusRatio;
                    painter->drawRoundedRect(option->rect, radius, radius);
                }
                else
                    painter->drawRect(option->rect);

                painter->restore();
            } else {
                if (vopt->backgroundBrush.style() != Qt::NoBrush) {
                    QPointF oldBO = painter->brushOrigin();
                    painter->setBrushOrigin(vopt->rect.topLeft());
                    painter->fillRect(vopt->rect, vopt->backgroundBrush);
                    painter->setBrushOrigin(oldBO);
                }

                if (vopt->state & QStyle::State_Selected) {
                    QRect textRect = subElementRect(QStyle::SE_ItemViewItemText,  option, widget);
                    painter->fillRect(textRect, vopt->palette.brush(cg, QPalette::Highlight));
                }
            }
        }
        break;
    }
    case PE_IndicatorDockWidgetResizeHandle:
    {
        QStyleOption dockWidgetHandle = *option;
        bool horizontal = option->state & State_Horizontal;
        dockWidgetHandle.state.setFlag(State_Horizontal, !horizontal);
        proxy()->drawControl(CE_Splitter, &dockWidgetHandle, painter, widget);
    }
        break;
    case PE_IndicatorItemViewItemDrop: {
        const qreal radius = frameRadius;
        QRect rect(option->rect.adjusted(radius, radius, -radius, -radius));
        if (rect.height() > 0) {
            painter->save();
            QColor color(option->palette.color(QPalette::Highlight));
            painter->setPen(color);
            painter->setBrush(QColor(color.red(), color.green(), color.blue(), 60));

            if (rect.height() > 0) {
                painter->drawRoundedRect(rect, radius, radius);
            }

            painter->restore();
        }
        break;
    }
    case PE_FrameWindow:
        painter->save();
    {
        painter->setPen(QPen(outline.darker(150)));
        painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
        painter->setPen(QPen(option->palette.light(), 1));
        painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1),
                          QPoint(rect.left() + 1, rect.bottom() - 1));
        painter->setPen(QPen(option->palette.window().color().darker(120)));
        painter->drawLine(QPoint(rect.left() + 1, rect.bottom() - 1),
                          QPoint(rect.right() - 2, rect.bottom() - 1));
        painter->drawLine(QPoint(rect.right() - 1, rect.top() + 1),
                          QPoint(rect.right() - 1, rect.bottom() - 1));
    }
        painter->restore();
        break;
    case PE_FrameLineEdit:
    {
        QRect r = rect;
        bool hasFocus = option->state & State_HasFocus;

        painter->save();

        painter->setRenderHint(QPainter::Antialiasing, true);
        //  ### highdpi painter bug.
        painter->translate(0.5, 0.5);

        // Draw Outline
        painter->setPen( QPen(hasFocus ? highlightedOutline : outline));
        painter->drawRoundedRect(r.adjusted(0, 0, -1, -1), 2, 2);

        if (hasFocus) {
            QColor softHighlight = highlightedOutline;
            softHighlight.setAlpha(40);
            painter->setPen(softHighlight);
            painter->drawRoundedRect(r.adjusted(1, 1, -2, -2), 1.7, 1.7);
        }
        // Draw inner shadow
        painter->setPen(topShadow);
        painter->drawLine(QPoint(r.left() + 2, r.top() + 1), QPoint(r.right() - 2, r.top() + 1));

        painter->restore();

    }
        break;
    case PE_IndicatorCheckBox:
        painter->save();
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);
            rect = rect.adjusted(0, 0, -1, -1);

            QColor pressedColor = mergedColors(option->palette.base().color(), option->palette.windowText().color(), 85);
            painter->setBrush(Qt::NoBrush);

            // Gradient fill
            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            gradient.setColorAt(0, (state & State_Sunken) ? pressedColor : option->palette.base().color().darker(115));
            gradient.setColorAt(0.15, (state & State_Sunken) ? pressedColor : option->palette.base().color());
            gradient.setColorAt(1, (state & State_Sunken) ? pressedColor : option->palette.base().color());

            painter->setBrush((state & State_Sunken) ? QBrush(pressedColor) : gradient);
            painter->setPen(QPen(outline.lighter(110)));

            if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange)
                painter->setPen(QPen(highlightedOutline));
            painter->drawRect(rect);

            QColor checkMarkColor = option->palette.text().color().darker(120);
            const qreal checkMarkPadding = 1 + rect.width() * 0.13; // at least one pixel padding

            if (checkbox->state & State_NoChange) {
                gradient = QLinearGradient(rect.topLeft(), rect.bottomLeft());
                checkMarkColor.setAlpha(80);
                gradient.setColorAt(0, checkMarkColor);
                checkMarkColor.setAlpha(140);
                gradient.setColorAt(1, checkMarkColor);
                checkMarkColor.setAlpha(180);
                painter->setPen(QPen(checkMarkColor, 1));
                painter->setBrush(gradient);
                painter->drawRect(rect.adjusted(checkMarkPadding, checkMarkPadding, -checkMarkPadding, -checkMarkPadding));

            } else if (checkbox->state & State_On) {
                const qreal dpi = StyleHelper::dpi(option);
                qreal penWidth = StyleHelper::dpiScaled(1.5, dpi);
                penWidth = qMax<qreal>(penWidth, 0.13 * rect.height());
                penWidth = qMin<qreal>(penWidth, 0.20 * rect.height());
                QPen checkPen = QPen(checkMarkColor, penWidth);
                checkMarkColor.setAlpha(210);
                painter->translate(StyleHelper::dpiScaled(-0.8, dpi), StyleHelper::dpiScaled(0.5, dpi));
                painter->setPen(checkPen);
                painter->setBrush(Qt::NoBrush);

                // Draw checkmark
                QPainterPath path;
                const qreal rectHeight = rect.height(); // assuming height equals width
                path.moveTo(checkMarkPadding + rectHeight * 0.11, rectHeight * 0.47);
                path.lineTo(rectHeight * 0.5, rectHeight - checkMarkPadding);
                path.lineTo(rectHeight - checkMarkPadding, checkMarkPadding);
                painter->drawPath(path.translated(rect.topLeft()));
            }
        }
        painter->restore();
        break;
    case PE_IndicatorRadioButton:
        painter->save();
    {
        QColor pressedColor = mergedColors(option->palette.base().color(), option->palette.windowText().color(), 85);
        painter->setBrush((state & State_Sunken) ? pressedColor : option->palette.base().color());
        painter->setRenderHint(QPainter::Antialiasing, true);
        QPainterPath circle;
        const QPointF circleCenter = rect.center() + QPoint(1, 1);
        const qreal outlineRadius = (rect.width() + (rect.width() + 1) % 2) / 2.0 - 1;
        circle.addEllipse(circleCenter, outlineRadius, outlineRadius);
        painter->setPen(QPen(option->palette.window().color().darker(150)));
        if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange)
            painter->setPen(QPen(highlightedOutline));
        painter->drawPath(circle);

        if (state & (State_On )) {
            circle = QPainterPath();
            const qreal checkmarkRadius = outlineRadius / 2.32;
            circle.addEllipse(circleCenter, checkmarkRadius, checkmarkRadius);
            QColor checkMarkColor = option->palette.text().color().darker(120);
            checkMarkColor.setAlpha(200);
            painter->setPen(checkMarkColor);
            checkMarkColor.setAlpha(180);
            painter->setBrush(checkMarkColor);
            painter->drawPath(circle);
        }
    }
        painter->restore();
        break;
    case PE_IndicatorToolBarHandle:
    {
        //draw grips
        if (option->state & State_Horizontal) {
            for (int i = -3 ; i < 2 ; i += 3) {
                for (int j = -8 ; j < 10 ; j += 3) {
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 2, 2, StyleHelper::lightShade());
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 1, 1, StyleHelper::darkShade());
                }
            }
        } else { //vertical toolbar
            for (int i = -6 ; i < 12 ; i += 3) {
                for (int j = -3 ; j < 2 ; j += 3) {
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 2, 2, StyleHelper::lightShade());
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 1, 1, StyleHelper::darkShade());
                }
            }
        }
        break;
    }
    case PE_FrameDefaultButton:
        break;
    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
            //### check for d->alt_down
            if (!(fropt->state & State_KeyboardFocusChange))
                return;
            QRect rect = option->rect;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);
            QColor fillcolor = highlightedOutline;
            fillcolor.setAlpha(80);
            painter->setPen(fillcolor.darker(120));
            fillcolor.setAlpha(30);
            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            gradient.setColorAt(0, fillcolor.lighter(160));
            gradient.setColorAt(1, fillcolor);
            painter->setBrush(gradient);
            painter->drawRoundedRect(option->rect.adjusted(0, 0, -1, -1), 1, 1);
            painter->restore();
        }
        break;
    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton* >(option)) {
            // rect and palette
            const auto &rect(option->rect);

            // button state
            const State &state(option->state);
            const bool enabled(state & State_Enabled);
            const bool mouseOver(enabled && (state & State_MouseOver));
            const bool hasFocus((enabled && (state & State_HasFocus)) && !(widget && widget->focusProxy()));
            const bool sunken(state & (State_On|State_Sunken));
            const bool flat(buttonOption->features & QStyleOptionButton::Flat);
            const qreal radius = option->rect.height() * m_radiusRatio;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);
            painter->fillRect(rect, Qt::transparent);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(242, 242, 242));
            const QRectF baseRect(rect.adjusted(0, 0, -radius / 2, -radius / 2));

            if (state & State_MouseOver) {
                if (sunken) {
                    // press
                    painter->setPen(option->palette.highlight().color());
                    painter->setBrush(QColor(204, 204, 204));
                } else {
                    // hover
                    painter->setBrush(QColor(224, 224, 224));
                }
            }

            // focus outline
            if (hasFocus) {
                painter->setPen(option->palette.color(QPalette::Highlight));
            }

            painter->drawRoundedRect(baseRect, radius, radius);
            painter->restore();
        }
        break;
    case PE_FrameTabWidget:
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(option->rect);
        painter->restore();
        break ;

    case PE_FrameStatusBarItem:
        break;
    // case PE_IndicatorTabClose:
    // {
    //     Q_D(const QFusionStyle);
    //     if (d->tabBarcloseButtonIcon.isNull())
    //         d->tabBarcloseButtonIcon = proxy()->standardIcon(SP_DialogCloseButton, option, widget);
    //     if ((option->state & State_Enabled) && (option->state & State_MouseOver))
    //         proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
    //     QPixmap pixmap = d->tabBarcloseButtonIcon.pixmap(qt_getWindow(widget), QSize(16, 16), QIcon::Normal, QIcon::On);
    //     proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
    // }
    //     break;
    // case PE_PanelMenu: {
    //     painter->save();
    //     const QBrush menuBackground = option->palette.base().color().lighter(108);
    //     QColor borderColor = option->palette.window().color().darker(160);
    //     qDrawPlainRect(painter, option->rect, borderColor, 1, &menuBackground);
    //     painter->restore();
    // }
    //     break;
    default:
        QCommonStyle::drawPrimitive(elem, option, painter, widget);
        break;
    }
}

void ModernStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                               const QWidget *widget) const
{
    QRect rect = option->rect;
    QColor outline = StyleHelper::outline(option->palette);
    QColor highlightedOutline = StyleHelper::highlightedOutline(option->palette);
    QColor shadow = StyleHelper::darkShade();

    switch (element) {
    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QRect editRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
            painter->save();
            painter->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
                QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal
                                                             : QIcon::Disabled;
                QPixmap pixmap = cb->currentIcon.pixmap(qt_getWindow(widget), cb->iconSize, mode);
                QRect iconRect(editRect);
                iconRect.setWidth(cb->iconSize.width() + 4);
                iconRect = alignedRect(cb->direction,
                                       Qt::AlignLeft | Qt::AlignVCenter,
                                       iconRect.size(), editRect);
                if (cb->editable)
                    painter->fillRect(iconRect, cb->palette.brush(QPalette::Base));
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                if (cb->direction == Qt::RightToLeft)
                    editRect.translate(-4 - cb->iconSize.width(), 0);
                else
                    editRect.translate(cb->iconSize.width() + 4, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable) {
                proxy()->drawItemText(painter, editRect.adjusted(1, 0, -1, 0),
                                      visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                      cb->palette, cb->state & State_Enabled, cb->currentText,
                                      cb->editable ? QPalette::Text : QPalette::ButtonText);
            }
            painter->restore();
        }
        break;
    case CE_ScrollBarSlider: {
        const QStyleOptionSlider *sliderOption(qstyleoption_cast<const QStyleOptionSlider *>(option));
        if (!sliderOption)
            break;

        const State &state(option->state);
        bool horizontal(state & State_Horizontal);

        // copy rect and palette
        const QRect &rect(horizontal ? option->rect.adjusted(-1, 4, 0, -4) : option->rect.adjusted(4, -1, -4, 0));
        //const QPalette &palette(option->palette);

        // define handle rect
        QRect handleRect;

        bool enabled(state & State_Enabled);
        bool mouseOver((state & State_Active) && enabled && (state & State_MouseOver));
        //bool sunken(enabled && (state & (State_On | State_Sunken)));
        qreal opacity;
        if (mouseOver)
            opacity = 0.7;
        else
            opacity = 0.2;

        if (horizontal) {
            handleRect = rect.adjusted(0, 6, 0, 2);
            handleRect.adjust(3, -6.0 * opacity, -6, -2.0 * opacity);
        } else {
            handleRect = rect.adjusted(6, 0, 2, 0);
            handleRect.adjust(-6.0 * opacity, 2, -2.0 * opacity, -4);
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        qreal metric(handleRect.width() < handleRect.height() ? handleRect.width() : handleRect.height());
        qreal radius(0.5 * metric);

        painter->setPen(Qt::NoPen);
        painter->setOpacity(opacity);
        painter->setBrush(option->palette.windowText());
        painter->drawRoundedRect(handleRect, radius, radius);
        painter->restore();
        break;
    }
    case CE_Splitter:
        {
            // Don't draw handle for single pixel splitters
            if (option->rect.width() > 1 && option->rect.height() > 1) {
                //draw grips
                if (option->state & State_Horizontal) {
                    for (int j = -6 ; j< 12 ; j += 3) {
                        painter->fillRect(rect.center().x() + 1, rect.center().y() + j, 2, 2, StyleHelper::lightShade());
                        painter->fillRect(rect.center().x() + 1, rect.center().y() + j, 1, 1, StyleHelper::darkShade());
                    }
                } else {
                    for (int i = -6; i< 12 ; i += 3) {
                        painter->fillRect(rect.center().x() + i, rect.center().y(), 2, 2, StyleHelper::lightShade());
                        painter->fillRect(rect.center().x() + i, rect.center().y(), 1, 1, StyleHelper::darkShade());
                    }
                }
            }
            break;
        }
    // rubberband
    case CE_RubberBand:
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(option)) {
            QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
            painter->save();
            QColor penColor = highlight.darker(120);
            penColor.setAlpha(180);
            painter->setPen(penColor);
            QColor dimHighlight(qMin(highlight.red()/2 + 110, 255),
                                qMin(highlight.green()/2 + 110, 255),
                                qMin(highlight.blue()/2 + 110, 255));
            dimHighlight.setAlpha(widget && widget->isTopLevel() ? 255 : 80);
            QLinearGradient gradient(rect.topLeft(), QPoint(rect.bottomLeft().x(), rect.bottomLeft().y()));
            gradient.setColorAt(0, dimHighlight.lighter(120));
            gradient.setColorAt(1, dimHighlight);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);
            painter->setBrush(dimHighlight);
            painter->drawRoundedRect(option->rect.adjusted(0, 0, -1, -1), 1, 1);
            QColor innerLine = Qt::white;
            innerLine.setAlpha(40);
            painter->setPen(innerLine);
            painter->drawRoundedRect(option->rect.adjusted(1, 1, -2, -2), 1, 1);
            painter->restore();
        }
        break;

    case CE_SizeGrip:
        painter->save();
    {
        //draw grips
        for (int i = -6; i< 12 ; i += 3) {
            for (int j = -6 ; j< 12 ; j += 3) {
                if ((option->direction == Qt::LeftToRight && i > -j) || (option->direction == Qt::RightToLeft && j > i) ) {
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 2, 2, StyleHelper::lightShade());
                    painter->fillRect(rect.center().x() + i, rect.center().y() + j, 1, 1, StyleHelper::darkShade());
                }
            }
        }
    }
        painter->restore();
        break;
    // toolbar
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolBar = qstyleoption_cast<const QStyleOptionToolBar *>(option)) {
            // Reserve the beveled appearance only for mainwindow toolbars
            if (widget && !(qobject_cast<const QMainWindow*> (widget->parentWidget())))
                break;

            // Draws the light line above and the dark line below menu bars and
            // tool bars.
            QLinearGradient gradient(option->rect.topLeft(), option->rect.bottomLeft());
            if (!(option->state & State_Horizontal))
                gradient = QLinearGradient(rect.left(), rect.center().y(),
                                           rect.right(), rect.center().y());
            gradient.setColorAt(0, option->palette.window().color().lighter(104));
            gradient.setColorAt(1, option->palette.window().color());
            painter->fillRect(option->rect, gradient);

            QColor light = StyleHelper::lightShade();
            QColor shadow = StyleHelper::darkShade();

            QPen oldPen = painter->pen();
            if (toolBar->toolBarArea == Qt::TopToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The end and onlyone top toolbar lines draw a double
                    // line at the bottom to blend with the central
                    // widget.
                    painter->setPen(light);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                } else {
                    // All others draw a single dark line at the bottom.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                // All top toolbar lines draw a light line at the top.
                painter->setPen(light);
                painter->drawLine(option->rect.topLeft(), option->rect.topRight());
            } else if (toolBar->toolBarArea == Qt::BottomToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::Middle) {
                    // The end and middle bottom tool bar lines draw a dark
                    // line at the bottom.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::Beginning
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The beginning and only one tool bar lines draw a
                    // double line at the bottom to blend with the
                    // status bar.
                    // ### The styleoption could contain whether the
                    // main window has a menu bar and a status bar, and
                    // possibly dock widgets.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                    painter->setPen(light);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                    painter->setPen(light);
                    painter->drawLine(option->rect.left(), option->rect.top() + 1,
                                      option->rect.right(), option->rect.top() + 1);

                } else {
                    // All other bottom toolbars draw a light line at the top.
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                }
            }
            if (toolBar->toolBarArea == Qt::LeftToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                        || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // The middle and left end toolbar lines draw a light
                    // line to the left.
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.right() - 1, option->rect.top(),
                                      option->rect.right() - 1, option->rect.bottom());
                    painter->setPen(light);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                } else {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
            } else if (toolBar->toolBarArea == Qt::RightToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                        || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // Right middle and end toolbar lines draw the dark right line
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The right end and single toolbar draws the dark
                    // line on its left edge
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                    // And a light line next to it
                    painter->setPen(light);
                    painter->drawLine(option->rect.left() + 1, option->rect.top(),
                                      option->rect.left() + 1, option->rect.bottom());
                } else {
                    // Other right toolbars draw a light line on its left edge
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
            }
            painter->setPen(oldPen);
        }
        break;

    case CE_DockWidgetTitle:
        painter->save();
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            bool verticalTitleBar = dwOpt->verticalTitleBar;

            QRect titleRect = subElementRect(SE_DockWidgetTitleBarText, option, widget);
            if (verticalTitleBar) {
                QRect rect = dwOpt->rect;
                QRect r = rect.transposed();
                titleRect = QRect(r.left() + rect.bottom()
                                  - titleRect.bottom(),
                                  r.top() + titleRect.left() - rect.left(),
                                  titleRect.height(), titleRect.width());

                painter->translate(r.left(), r.top() + r.width());
                painter->rotate(-90);
                painter->translate(-r.left(), -r.top());
            }

            if (!dwOpt->title.isEmpty()) {
                QString titleText
                        = painter->fontMetrics().elidedText(dwOpt->title,
                                                            Qt::ElideRight, titleRect.width());
                proxy()->drawItemText(painter,
                                      titleRect,
                                      Qt::AlignLeft | Qt::AlignVCenter, dwOpt->palette,
                                      dwOpt->state & State_Enabled, titleText,
                                      QPalette::WindowText);
            }
        }
        painter->restore();
        break;
    case CE_HeaderSection: {
        const auto headerOption(qstyleoption_cast<const QStyleOptionHeader*>(option));
        if (!headerOption) return;
        const bool horizontal(headerOption->orientation == Qt::Horizontal);
        const bool isLast(headerOption->position == QStyleOptionHeader::End);

        // draw background
        painter->fillRect(option->rect, QBrush(option->palette.alternateBase().color()));

        // draw line
        QColor lineColor(option->palette.alternateBase().color().darker(110));
        painter->setPen(lineColor);
        if (horizontal) {
            if (!isLast) {
                QPoint unit(0, option->rect.height() / 5);
                painter->drawLine(option->rect.topRight() + unit, option->rect.bottomRight() - unit);
            }
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        } else {
            if (!isLast) {
                painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
            }
            painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
        }
        break;
    }
    case CE_ProgressBarGroove:
        painter->save();
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0.5, 0.5);

        QColor shadowAlpha = Qt::black;
        shadowAlpha.setAlpha(16);
        painter->setPen(shadowAlpha);
        painter->drawLine(rect.topLeft() - QPoint(0, 1), rect.topRight() - QPoint(0, 1));

        painter->setBrush(option->palette.base());
        painter->setPen(QPen(outline));
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 2, 2);

        // Inner shadow
        painter->setPen(StyleHelper::topShadow());
        painter->drawLine(QPoint(rect.left() + 1, rect.top() + 1),
                          QPoint(rect.right() - 1, rect.top() + 1));
    }
        painter->restore();
        break;
    case CE_ProgressBarContents:
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0.5, 0.5);
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            bool vertical = false;
            bool inverted = false;
            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);
            bool complete = bar->progress == bar->maximum;

            // Get extra style options if version 2
            vertical = (bar->orientation == Qt::Vertical);
            inverted = bar->invertedAppearance;

            // If the orientation is vertical, we use a transform to rotate
            // the progress bar 90 degrees clockwise.  This way we can use the
            // same rendering code for both orientations.
            if (vertical) {
                rect = QRect(rect.left(), rect.top(), rect.height(), rect.width()); // flip width and height
                QTransform m = QTransform::fromTranslate(rect.height()-1, -1.0);
                m.rotate(90.0);
                painter->setTransform(m, true);
            }

            int maxWidth = rect.width();
            const auto progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
            const auto totalSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
            const auto progressSteps = qint64(progress) - bar->minimum;
            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
            int width = indeterminate ? maxWidth : progressBarWidth;

            bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;
            if (inverted)
                reverse = !reverse;

            int step = 0;
            QRect progressBar;
            QColor highlight = StyleHelper::highlight(option->palette);
            QColor highlightedoutline = highlight.darker(140);
            if (qGray(outline.rgb()) > qGray(highlightedoutline.rgb()))
                outline = highlightedoutline;

            if (!indeterminate) {
                QColor innerShadow(Qt::black);
                innerShadow.setAlpha(35);
                painter->setPen(innerShadow);
                if (!reverse) {
                    progressBar.setRect(rect.left(), rect.top(), width - 1, rect.height() - 1);
                    if (!complete) {
                        painter->drawLine(progressBar.topRight() + QPoint(2, 1), progressBar.bottomRight() + QPoint(2, 0));
                        painter->setPen(QPen(highlight.darker(140)));
                        painter->drawLine(progressBar.topRight() + QPoint(1, 1), progressBar.bottomRight() + QPoint(1, 0));
                    }
                } else {
                    progressBar.setRect(rect.right() - width - 1, rect.top(), width + 2, rect.height() - 1);
                    if (!complete) {
                        painter->drawLine(progressBar.topLeft() + QPoint(-2, 1), progressBar.bottomLeft() + QPoint(-2, 0));
                        painter->setPen(QPen(highlight.darker(140)));
                        painter->drawLine(progressBar.topLeft() + QPoint(-1, 1), progressBar.bottomLeft() + QPoint(-1, 0));
                    }
                }
            } else {
                progressBar.setRect(rect.left(), rect.top(), rect.width() - 1, rect.height() - 1);
            }

            if (indeterminate || bar->progress > bar->minimum) {

                painter->setPen(QPen(outline));

                QColor highlightedGradientStartColor = highlight.lighter(120);
                QColor highlightedGradientStopColor  = highlight;
                QLinearGradient gradient(rect.topLeft(), QPoint(rect.bottomLeft().x(), rect.bottomLeft().y()));
                gradient.setColorAt(0, highlightedGradientStartColor);
                gradient.setColorAt(1, highlightedGradientStopColor);

                painter->setBrush(gradient);

                painter->save();
                if (!complete && !indeterminate)
                    painter->setClipRect(progressBar.adjusted(-1, -1, -1, 1));
                QRect fillRect = progressBar.adjusted( !indeterminate && !complete && reverse ? -2 : 0, 0,
                                                       indeterminate || complete || reverse ? 0 : 2, 0);
                painter->drawRoundedRect(fillRect, 2, 2);
                painter->restore();

                painter->setBrush(Qt::NoBrush);
                painter->setPen(QColor(255, 255, 255, 50));
                painter->drawRoundedRect(progressBar.adjusted(1, 1, -1, -1), 1, 1);

                if (!indeterminate) {
// #if QT_CONFIG(animation)
//                     (const_cast<QFusionStylePrivate*>(d))->stopAnimation(option->styleObject);
// #endif
                } else {
                    highlightedGradientStartColor.setAlpha(120);
                    painter->setPen(QPen(highlightedGradientStartColor, 9.0));
                    painter->setClipRect(progressBar.adjusted(1, 1, -1, -1));
// #if QT_CONFIG(animation)
//                 if (QProgressStyleAnimation *animation = qobject_cast<QProgressStyleAnimation*>(d->animation(option->styleObject)))
//                     step = animation->animationStep() % 22;
//                 else
//                     (const_cast<QFusionStylePrivate*>(d))->startAnimation(new QProgressStyleAnimation(d->animationFps, option->styleObject));
// #endif
                for (int x = progressBar.left() - rect.height(); x < rect.right() ; x += 22)
                    painter->drawLine(x + step, progressBar.bottom() + 1,
                                      x + rect.height() + step, progressBar.top() - 2);
                }
            }
        }
        painter->restore();
        break;
    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QRect leftRect;
            QRect rect = bar->rect;
            QColor textColor = option->palette.text().color();
            QColor alternateTextColor = StyleHelper::highlightedText(option->palette);

            painter->save();
            bool vertical = false, inverted = false;
            vertical = (bar->orientation == Qt::Vertical);
            inverted = bar->invertedAppearance;
            if (vertical)
                rect = QRect(rect.left(), rect.top(), rect.height(), rect.width()); // flip width and height
            const auto totalSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
            const auto progressSteps = qint64(bar->progress) - bar->minimum;
            const auto progressIndicatorPos = progressSteps * rect.width() / totalSteps;
            if (progressIndicatorPos >= 0 && progressIndicatorPos <= rect.width())
                leftRect = QRect(rect.left(), rect.top(), progressIndicatorPos, rect.height());
            if (vertical)
                leftRect.translate(rect.width() - progressIndicatorPos, 0);

            bool flip = (!vertical && (((bar->direction == Qt::RightToLeft) && !inverted) ||
                                       ((bar->direction == Qt::LeftToRight) && inverted)));

            QRegion rightRect = rect;
            rightRect = rightRect.subtracted(leftRect);
            painter->setClipRegion(rightRect);
            painter->setPen(flip ? alternateTextColor : textColor);
            painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            if (!leftRect.isNull()) {
                painter->setPen(flip ? textColor : alternateTextColor);
                painter->setClipRect(leftRect);
                painter->drawText(rect, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            }
            painter->restore();
        }
        break;
    case CE_MenuBarItem:
        painter->save();
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect.adjusted(0, 1, 0, -3);
            QColor highlightOutline = option->palette.highlight().color().darker(125);
            painter->fillRect(rect, option->palette.window());

            QCommonStyle::drawControl(element, &item, painter, widget);

            bool act = mbi->state & State_Selected && mbi->state & State_Sunken;
            bool dis = !(mbi->state & State_Enabled);

            QRect r = option->rect;
            if (act) {
                painter->setBrush(option->palette.highlight().color());
                painter->setPen(QPen(highlightOutline));
                painter->drawRect(r.adjusted(0, 0, -1, -1));

                //                painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 2, 2);

                //draw text
                QPalette::ColorRole textRole = dis ? QPalette::Text : QPalette::HighlightedText;
                uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!proxy()->styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                proxy()->drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            } else {

                QColor shadow = mergedColors(option->palette.window().color().darker(120),
                                             outline.lighter(140), 60);
                painter->setPen(QPen(shadow));
                painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
            }
        }
        painter->restore();
        break;
    case CE_MenuItem:
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        // Draws one item in a popup menu.
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            QColor highlightOutline = highlightedOutline;
            QColor highlight = option->palette.highlight().color();
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                int w = 0;
                const int margin = int(StyleHelper::dpiScaled(5, option));
                if (!menuItem->text.isEmpty()) {
                    painter->setFont(menuItem->font);
                    proxy()->drawItemText(painter, menuItem->rect.adjusted(margin, 0, -margin, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                          menuItem->palette, menuItem->state & State_Enabled, menuItem->text,
                                          QPalette::Text);
                    w = menuItem->fontMetrics.horizontalAdvance(menuItem->text) + margin;
                }
                painter->setPen(shadow.lighter(106));
                bool reverse = menuItem->direction == Qt::RightToLeft;
                painter->drawLine(menuItem->rect.left() + margin + (reverse ? 0 : w), menuItem->rect.center().y(),
                                  menuItem->rect.right() - margin - (reverse ? w : 0), menuItem->rect.center().y());
                painter->restore();
                break;
            }
            bool selected = menuItem->state & State_Selected && menuItem->state & State_Enabled;
            if (selected) {
                const QRect r = option->rect;
                const int radius = r.height() * 0.2;
                painter->setBrush(highlight);
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
            }
            bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
            bool checked = menuItem->checked;
            bool sunken = menuItem->state & State_Sunken;
            bool enabled = menuItem->state & State_Enabled;

            bool ignoreCheckMark = false;
            const int checkColHOffset = windowsItemHMargin + windowsItemFrame - 1;
            int checkcol = qMax<int>(menuItem->rect.height() * 0.79,
                                     qMax<int>(menuItem->maxIconWidth, StyleHelper::dpiScaled(21, option))); // icon checkbox's highlight column width
            if (
//#if QT_CONFIG(combobox)
                qobject_cast<const QComboBox*>(widget) ||
//#endif
                (option->styleObject && option->styleObject->property("_q_isComboBoxPopupItem").toBool()))
                ignoreCheckMark = true; //ignore the checkmarks provided by the QComboMenuDelegate

            if (!ignoreCheckMark || menuItem->state & (State_On | State_Off)) {
                // Check, using qreal and QRectF to avoid error accumulation
                const qreal boxMargin = StyleHelper::dpiScaled(3.5, option);
                const qreal boxWidth = checkcol - 2 * boxMargin;
                QRectF checkRectF(option->rect.left() + boxMargin + checkColHOffset, option->rect.center().y() - boxWidth/2 + 1, boxWidth, boxWidth);
                QRect checkRect = checkRectF.toRect();
                checkRect.setWidth(checkRect.height()); // avoid .toRect() round error results in non-perfect square
                checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
                if (checkable) {
                    if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
                        // Radio button
                        if (menuItem->state & State_On || checked || sunken) {
                            painter->setRenderHint(QPainter::Antialiasing);
                            painter->setPen(Qt::NoPen);

                            QPalette::ColorRole textRole = !enabled ? QPalette::Text:
                                                                      selected ? QPalette::HighlightedText : QPalette::ButtonText;
                            painter->setBrush(option->palette.brush( option->palette.currentColorGroup(), textRole));
                            const int adjustment = checkRect.height() * 0.3;
                            painter->drawEllipse(checkRect.adjusted(adjustment, adjustment, -adjustment, -adjustment));
                        }
                    } else {
                        // Check box
                        if (menuItem->icon.isNull()) {
                            QStyleOptionButton box;
                            box.QStyleOption::operator=(*option);
                            box.rect = checkRect;
                            if (checked || menuItem->state & State_On)
                                box.state |= State_On;
                            else
                                box.state |= State_Off;
                            proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
                        }
                    }
                }
            } else { //ignore checkmark
                if (menuItem->icon.isNull())
                    checkcol = 0;
                else
                    checkcol = menuItem->maxIconWidth;
            }

            // Text and icon, ripped from windows style
            bool dis = !(menuItem->state & State_Enabled);
            bool act = menuItem->state & State_Selected;
            const QStyleOption *opt = option;
            const QStyleOptionMenuItem *menuitem = menuItem;

            QPainter *p = painter;
            QRect vCheckRect = visualRect(opt->direction, menuitem->rect,
                                          QRect(menuitem->rect.x() + checkColHOffset, menuitem->rect.y(),
                                                checkcol, menuitem->rect.height()));
            if (!menuItem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;

                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                QSize iconSize(smallIconSize, smallIconSize);
//#if QT_CONFIG(combobox)
                if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget))
                    iconSize = combo->iconSize();
//#endif
                if (checked)
                    pixmap = menuItem->icon.pixmap(qt_getWindow(widget), iconSize, mode, QIcon::On);
                else
                    pixmap = menuItem->icon.pixmap(qt_getWindow(widget), iconSize, mode);

                const int pixw = pixmap.width() / pixmap.devicePixelRatio();
                const int pixh = pixmap.height() / pixmap.devicePixelRatio();

                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                painter->setPen(menuItem->palette.text().color());
                if (!ignoreCheckMark && checkable && checked) {
                    QStyleOption opt = *option;
                    if (act) {
                        QColor activeColor = mergedColors(option->palette.window().color(),
                                                          option->palette.highlight().color());
                        opt.palette.setBrush(QPalette::Button, activeColor);
                    }
                    opt.state |= State_Sunken;
                    opt.rect = vCheckRect;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &opt, painter, widget);
                }
                painter->drawPixmap(pmr.topLeft(), pixmap);
            }
            if (selected) {
                painter->setPen(menuItem->palette.highlightedText().color());
            } else {
                painter->setPen(menuItem->palette.text().color());
            }
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }
            int xm = checkColHOffset + checkcol + windowsItemHMargin;
            int xpos = menuitem->rect.x() + xm;

            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QStringRef s(&menuitem->text);
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf(QLatin1Char('\t'));
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!proxy()->styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect,
                                                     QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
                    const QString textToDraw = s.mid(t + 1).toString();
                    if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, option, widget)) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1, 1, 1, 1), text_flags, textToDraw);
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, textToDraw);
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                // font may not have any "hard" flags set. We override
                // the point size so that when it is resolved against the device, this font will win.
                // This is mainly to handle cases where someone sets the font on the window
                // and then the combo inherits it and passes it onward. At that point the resolve mask
                // is very, very weak. This makes it stonger.
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());

                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);

                p->setFont(font);
                const QString textToDraw = s.left(t).toString();
                if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, option, widget)) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1, 1, 1, 1), text_flags, textToDraw);
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, textToDraw);
                p->restore();
            }

            // Arrow
            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (menuItem->rect.height() - 4) / 2;
                PrimitiveElement arrow;
                arrow = option->direction == Qt::RightToLeft ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                int xpos = menuItem->rect.left() + menuItem->rect.width() - 3 - dim;
                QRect  vSubMenuRect = visualRect(option->direction, menuItem->rect,
                                                 QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuItem;
                newMI.rect = vSubMenuRect;
                newMI.state = !enabled ? State_None : State_Enabled;
                if (selected)
                    newMI.palette.setColor(QPalette::WindowText,
                                           newMI.palette.highlightedText().color());
                proxy()->drawPrimitive(arrow, &newMI, painter, widget);
            }
        }
        painter->restore();
        break;
    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            painter->save();
            painter->setClipRect(option->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

            // draw the background
            proxy()->drawPrimitive(PE_PanelItemViewItem, option, painter, widget);

            // draw the check mark
            if (vopt->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionViewItem option(*vopt);
                option.rect = checkRect;
                option.state = option.state & ~QStyle::State_HasFocus;

                switch (vopt->checkState) {
                case Qt::Unchecked:
                    option.state |= QStyle::State_Off;
                    break;
                case Qt::PartiallyChecked:
                    option.state |= QStyle::State_NoChange;
                    break;
                case Qt::Checked:
                    option.state |= QStyle::State_On;
                    break;
                }
                proxy()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, painter, widget);
            }

            // draw the icon
            QIcon::Mode mode = QIcon::Normal;
            if (!(vopt->state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (vopt->state & QStyle::State_Selected)
                mode = QIcon::Selected;
            QIcon::State state = vopt->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            vopt->icon.paint(painter, iconRect, vopt->decorationAlignment, mode, state);

            // draw the text
            if (!vopt->text.isEmpty()) {
                QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if (vopt->state & QStyle::State_Selected) {
                    painter->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                } else {
                    painter->setPen(vopt->palette.color(cg, QPalette::Text));
                }
                if (vopt->state & QStyle::State_Editing) {
                    painter->setPen(vopt->palette.color(cg, QPalette::Text));
                    painter->drawRect(textRect.adjusted(0, 0, -1, -1));
                }

                viewItemDrawText(painter, vopt, textRect);
            }

            // NO FOCUS
            painter->restore();
        }
        break;
    case CE_MenuHMargin:
    case CE_MenuVMargin:
        break;
    case CE_MenuEmptyArea:
        break;
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton b(*button);
            // no PM_ButtonShiftHorizontal and PM_ButtonShiftVertical for fusion style
            b.state &= ~(State_On | State_Sunken);
            QCommonStyle::drawControl(element, &b, painter, widget);
        }
        break;
    case CE_MenuBarEmptyArea:
        painter->save();
    {
        painter->fillRect(rect, option->palette.window());
        QColor shadow = mergedColors(option->palette.window().color().darker(120),
                                     outline.lighter(140), 60);
        painter->setPen(QPen(shadow));
        painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
    }
        painter->restore();
        break;
    // tabbar
    case CE_TabBarTabShape:
        painter->save();
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
            rect = option->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

            QRect r2(rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();

            painter->setPen(StyleHelper::innerContrastLine());

            QTransform rotMatrix;
            bool flip = false;
            painter->setPen(shadow);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedWest:
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedEast:
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - rect.width() + 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                QRect tmp = rect;
                rect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
                int temp = x1;
                x1 = y1;
                y1 = temp;
                temp = x2;
                x2 = y2;
                y2 = temp;
            }

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);

//            QColor tabFrameColor = tab->features & QStyleOptionTab::HasFrame ?
//                        StyleHelper::tabFrameColor(option->palette) :
//                        option->palette.window().color();

//            QLinearGradient fillGradient(rect.topLeft(), rect.bottomLeft());
//            QLinearGradient outlineGradient(rect.topLeft(), rect.bottomLeft());
//            QPen outlinePen = outline.lighter(110);
//            if (selected) {
//                fillGradient.setColorAt(0, tabFrameColor.lighter(104));
//                fillGradient.setColorAt(1, tabFrameColor);
//                outlineGradient.setColorAt(1, outline);
//                outlinePen = QPen(outlineGradient, 1);
//            } else {
//                fillGradient.setColorAt(0, tabFrameColor.darker(108));
//                fillGradient.setColorAt(0.85, tabFrameColor.darker(108));
//                fillGradient.setColorAt(1, tabFrameColor.darker(116));
//            }

//            QRect drawRect = rect.adjusted(0, selected ? 0 : 2, 0, 3);
//            painter->setPen(outlinePen);
//            painter->save();
//            painter->setClipRect(rect.adjusted(-1, -1, 1, selected ? -2 : -3));
//            painter->setBrush(fillGradient);
//            painter->drawRoundedRect(drawRect.adjusted(0, 0, -1, -1), 2.0, 2.0);
//            painter->setBrush(Qt::NoBrush);
//            painter->setPen(StyleHelper::innerContrastLine());
//            painter->drawRoundedRect(drawRect.adjusted(1, 1, -2, -1), 2.0, 2.0);
//            painter->restore();

            QRect drawRect = rect;
            painter->setPen(Qt::NoPen);
            painter->setBrush(selected ? option->palette.highlight().color().lighter(100) : Qt::transparent);

            if (selected) {
                const int radius = drawRect.height() * m_radiusRatio;
                painter->drawRoundedRect(drawRect.adjusted(radius, radius, -radius, -radius), radius, radius);
                // painter->fillRect(rect.left() + 1, rect.bottom() - 1, rect.width() - 2, rect.bottom() - 1, tabFrameColor);
                // painter->fillRect(QRect(rect.bottomRight() + QPoint(-2, -1), QSize(1, 1)), StyleHelper::innerContrastLine());
                // painter->fillRect(QRect(rect.bottomLeft() + QPoint(0, -1), QSize(1, 1)), StyleHelper::innerContrastLine());
                // painter->fillRect(QRect(rect.bottomRight() + QPoint(-1, -1), QSize(1, 1)), StyleHelper::innerContrastLine());
            }
        }
        painter->restore();
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect tr = tab->rect;
            bool verticalTabs = tab->shape == QTabBar::RoundedEast
                                || tab->shape == QTabBar::RoundedWest
                                || tab->shape == QTabBar::TriangularEast
                                || tab->shape == QTabBar::TriangularWest;

            int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextHideMnemonic;

            if (verticalTabs) {
                painter->save();
                int newX, newY, newRot;
                if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    newX = tr.width() + tr.x();
                    newY = tr.y();
                    newRot = 90;
                } else {
                    newX = tr.x();
                    newY = tr.y() + tr.height();
                    newRot = -90;
                }
                QTransform m = QTransform::fromTranslate(newX, newY);
                m.rotate(newRot);
                painter->setTransform(m, true);
            }
            QRect iconRect;
            tabLayout(tab, widget, &tr, &iconRect);
            tr = proxy()->subElementRect(SE_TabBarTabText, option, widget); //we compute tr twice because the style may override subElementRect

            if (!tab->icon.isNull()) {
                QPixmap tabIcon = tab->icon.pixmap(qt_getWindow(widget), tab->iconSize,
                                                   (tab->state & State_Enabled) ? QIcon::Normal
                                                                                : QIcon::Disabled,
                                                   (tab->state & State_Selected) ? QIcon::On
                                                                                 : QIcon::Off);
                painter->drawPixmap(iconRect.x(), iconRect.y(), tabIcon);
            }

            bool selected = tab->state & State_Selected && tab->state & State_Enabled;
            proxy()->drawItemText(painter, tr, alignment, tab->palette, tab->state & State_Enabled, tab->text, selected ? QPalette::HighlightedText : QPalette::WindowText);
            if (verticalTabs)
                painter->restore();

//            if (tab->state & State_HasFocus) {
//                const int OFFSET = 1 + pixelMetric(PM_DefaultFrameWidth);

//                int x1, x2;
//                x1 = tab->rect.left();
//                x2 = tab->rect.right() - 1;

//                QStyleOptionFocusRect fropt;
//                fropt.QStyleOption::operator=(*tab);
//                fropt.rect.setRect(x1 + 1 + OFFSET, tab->rect.y() + OFFSET,
//                                   x2 - x1 - 2*OFFSET, tab->rect.height() - 2*OFFSET);
//                drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
//            }
        }
        break;
    default:
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    }
}

int ModernStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    int val = -1;
    switch (metric) {
    case PM_SliderTickmarkOffset:
        val = 4;
        break;
    case PM_HeaderMargin:
    case PM_ToolTipLabelFrameWidth:
        val = 2;
        break;
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        val = 0;
        break;
    case PM_MessageBoxIconSize:
        val = 48;
        break;
    case PM_ListViewIconSize:
        val = 24;
        break;
    case PM_DialogButtonsSeparator:
    case PM_ScrollBarSliderMin:
        val = 26;
        break;
    case PM_TitleBarHeight:
        val = 24;
        break;
    case PM_ScrollBarExtent:
        val = 14;
        break;
    case PM_SliderThickness:
    case PM_SliderLength:
        val = 15;
        break;
    case PM_DockWidgetTitleMargin:
        val = 1;
        break;
    case PM_SpinBoxFrameWidth:
        val = 3;
        break;

    // menu
    case PM_MenuVMargin:
        val = frameRadius + 5;
        break;
    case PM_MenuHMargin:
        val = frameRadius + 5;
        break;
    case PM_MenuPanelWidth:
        val = 0;
        break;
    case PM_MenuBarItemSpacing:
        val = 6;
        break;
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:
    case PM_MenuBarPanelWidth:
        val = 0;
        break;
    case PM_ToolBarHandleExtent:
        val = 9;
        break;
    case PM_ToolBarItemSpacing:
        val = 1;
        break;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarItemMargin:
        val = 2;
        break;
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
        val = 16;
        break;
    case PM_DockWidgetTitleBarButtonMargin:
        val = 2;
        break;
    case PM_TitleBarButtonSize:
        val = 19;
        break;
    case PM_MaximumDragDistance:
        return -1; // Do not dpi-scale because the value is magic
    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        val = 20;
        break;
    case PM_TabBarTabVSpace:
        val = 12;
        break;
    case PM_TabBarTabOverlap:
        val = 1;
        break;
    case PM_TabBarBaseOverlap:
        val = 2;
        break;
    case PM_SubMenuOverlap:
        val = -1;
        break;
    case PM_DockWidgetHandleExtent:
    case PM_SplitterWidth:
        val = splitterWidth;
        break;
    case PM_IndicatorHeight:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
        val = 14;
        break;
    case PM_ScrollView_ScrollBarSpacing:
        val = 0;
        break;
    case PM_ScrollView_ScrollBarOverlap:
        if (proxy()->styleHint(SH_ScrollBar_Transient, option, widget))
            return proxy()->pixelMetric(PM_ScrollBarExtent, option, widget);
        val = 0;
        break;
    case PM_DefaultFrameWidth:
        return 1; // Do not dpi-scale because the drawn frame is always exactly 1 pixel thick
    default:
        return QCommonStyle::pixelMetric(metric, option, widget);
    }
    return StyleHelper::dpiScaled(val, option);
}

void ModernStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option,
                                      QPainter *painter, const QWidget *widget) const
{
    QColor buttonColor = StyleHelper::buttonColor(option->palette);
    QColor gradientStopColor = buttonColor;
    QColor gradientStartColor = buttonColor.lighter(118);
    QColor outline = StyleHelper::outline(option->palette);

    QColor alphaCornerColor;
    if (widget) {
        // ### backgroundrole/foregroundrole should be part of the style option
        alphaCornerColor = mergedColors(option->palette.color(widget->backgroundRole()), outline);
    } else {
        alphaCornerColor = mergedColors(option->palette.window().color(), outline);
    }

    switch (control) {
    case CC_ScrollBar: {
        bool enabled(option->state & State_Enabled);
        bool mouseOver((option->state & State_Active) && option->state & State_MouseOver);
        // render full groove directly, rather than using the addPage and subPage control element methods
        if (mouseOver && option->subControls & SC_ScrollBarGroove) {
            if (enabled) {
                painter->setPen(Qt::NoPen);
                // painter->setBrush(QColor(255, 255, 255, 150));
                painter->setBrush(Qt::transparent);
                painter->drawRect(option->rect);
            }
        }

        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QStyleOptionSlider newScrollbar = *scrollbar;
            State saveFlags = scrollbar->state;

            if (scrollbar->subControls & SC_ScrollBarSlider) {
                            newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = proxy()->subControlRect(CC_ScrollBar, &newScrollbar, SC_ScrollBarSlider, widget);
                if (newScrollbar.rect.isValid()) {
                    proxy()->drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);

                    if (scrollbar->state & State_HasFocus) {
                        QStyleOptionFocusRect fropt;
                        fropt.QStyleOption::operator=(newScrollbar);
                        fropt.rect.setRect(newScrollbar.rect.x() + 2, newScrollbar.rect.y() + 2,
                                        newScrollbar.rect.width() - 5,
                                        newScrollbar.rect.height() - 5);
                        proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                    }
                }
            }
        }
        break;
    }
    case CC_GroupBox:
        painter->save();
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            // Draw frame
            QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);

            if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                QStyleOptionFrame frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
                proxy()->drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                // groupBox->textColor gets the incorrect palette here
                painter->setPen(QPen(option->palette.windowText(), 1));
                int alignment = int(groupBox->textAlignment);
                if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                proxy()->drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignLeft | alignment,
                                      groupBox->palette, groupBox->state & State_Enabled, groupBox->text, QPalette::NoRole);

                if (groupBox->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect.adjusted(-2, -1, 2, 1);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }
        }
        painter->restore();
        break;

    case CE_ScrollBarSlider: {
        const QStyleOptionSlider *sliderOption(qstyleoption_cast<const QStyleOptionSlider *>(option));
        if (!sliderOption)
            break;

        const State &state(option->state);
        bool horizontal(state & State_Horizontal);

        // copy rect and palette
        const QRect &rect(horizontal ? option->rect.adjusted(-1, 4, 0, -4) : option->rect.adjusted(4, -1, -4, 0));
        //const QPalette &palette(option->palette);

        // define handle rect
        QRect handleRect;

        bool enabled(state & State_Enabled);
        bool mouseOver((state & State_Active) && enabled && (state & State_MouseOver));
        //bool sunken(enabled && (state & (State_On | State_Sunken)));
        qreal opacity;
        if (mouseOver)
            opacity = 0.7;
        else
            opacity = 0.2;

        if (horizontal) {
            handleRect = rect.adjusted(0, 6, 0, 2);
            handleRect.adjust(3, -6.0 * opacity, -6, -2.0 * opacity);
        } else {
            handleRect = rect.adjusted(6, 0, 2, 0);
            handleRect.adjust(-6.0 * opacity, 2, -2.0 * opacity, -4);
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        qreal metric(handleRect.width() < handleRect.height() ? handleRect.width() : handleRect.height());
        qreal radius(0.5 * metric);

        painter->setPen(Qt::NoPen);
        painter->setOpacity(opacity);
        painter->setBrush(option->palette.windowText());
        painter->drawRoundedRect(handleRect, radius, radius);
        painter->restore();
        break;
    }
    // case CC_SpinBox:
    //     if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
    //         QPixmap cache;
    //         QString pixmapName = QStyleHelper::uniqueName(QLatin1String("spinbox"), spinBox, spinBox->rect.size());
    //         if (!QPixmapCache::find(pixmapName, &cache)) {

    //             cache = styleCachePixmap(spinBox->rect.size());
    //             cache.fill(Qt::transparent);

    //             QRect pixmapRect(0, 0, spinBox->rect.width(), spinBox->rect.height());
    //             QRect rect = pixmapRect;
    //             QRect r = rect.adjusted(0, 1, 0, -1);
    //             QPainter cachePainter(&cache);
    //             QColor arrowColor = spinBox->palette.windowText().color();
    //             arrowColor.setAlpha(160);

    //             bool isEnabled = (spinBox->state & State_Enabled);
    //             bool hover = isEnabled && (spinBox->state & State_MouseOver);
    //             bool sunken = (spinBox->state & State_Sunken);
    //             bool upIsActive = (spinBox->activeSubControls == SC_SpinBoxUp);
    //             bool downIsActive = (spinBox->activeSubControls == SC_SpinBoxDown);
    //             bool hasFocus = (option->state & State_HasFocus);

    //             QStyleOptionSpinBox spinBoxCopy = *spinBox;
    //             spinBoxCopy.rect = pixmapRect;
    //             QRect upRect = proxy()->subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxUp, widget);
    //             QRect downRect = proxy()->subControlRect(CC_SpinBox, &spinBoxCopy, SC_SpinBoxDown, widget);

    //             if (spinBox->frame) {
    //                 cachePainter.save();
    //                 cachePainter.setRenderHint(QPainter::Antialiasing, true);
    //                 cachePainter.translate(0.5, 0.5);

    //                 // Fill background
    //                 cachePainter.setPen(Qt::NoPen);
    //                 cachePainter.setBrush(option->palette.base());
    //                 cachePainter.drawRoundedRect(r.adjusted(0, 0, -1, -1), 2, 2);

    //                 // Draw inner shadow
    //                 cachePainter.setPen(d->topShadow());
    //                 cachePainter.drawLine(QPoint(r.left() + 2, r.top() + 1), QPoint(r.right() - 2, r.top() + 1));

    //                 // Draw button gradient
    //                 QColor buttonColor = d->buttonColor(option->palette);
    //                 QRect updownRect = upRect.adjusted(0, -2, 0, downRect.height() + 2);
    //                 QLinearGradient gradient = qt_fusion_gradient(updownRect, (isEnabled && option->state & State_MouseOver ) ? buttonColor : buttonColor.darker(104));

    //                 // Draw button gradient
    //                 cachePainter.setPen(Qt::NoPen);
    //                 cachePainter.setBrush(gradient);

    //                 cachePainter.save();
    //                 cachePainter.setClipRect(updownRect);
    //                 cachePainter.drawRoundedRect(r.adjusted(0, 0, -1, -1), 2, 2);
    //                 cachePainter.setPen(QPen(d->innerContrastLine()));
    //                 cachePainter.setBrush(Qt::NoBrush);
    //                 cachePainter.drawRoundedRect(r.adjusted(1, 1, -2, -2), 2, 2);
    //                 cachePainter.restore();

    //                 if ((spinBox->stepEnabled & QAbstractSpinBox::StepUpEnabled) && upIsActive) {
    //                     if (sunken)
    //                         cachePainter.fillRect(upRect.adjusted(0, -1, 0, 0), gradientStopColor.darker(110));
    //                     else if (hover)
    //                         cachePainter.fillRect(upRect.adjusted(0, -1, 0, 0), d->innerContrastLine());
    //                 }

    //                 if ((spinBox->stepEnabled & QAbstractSpinBox::StepDownEnabled) && downIsActive) {
    //                     if (sunken)
    //                         cachePainter.fillRect(downRect.adjusted(0, 0, 0, 1), gradientStopColor.darker(110));
    //                     else if (hover)
    //                         cachePainter.fillRect(downRect.adjusted(0, 0, 0, 1), d->innerContrastLine());
    //                 }

    //                 cachePainter.setPen(hasFocus ? d->highlightedOutline(option->palette) : outline);
    //                 cachePainter.setBrush(Qt::NoBrush);
    //                 cachePainter.drawRoundedRect(r.adjusted(0, 0, -1, -1), 2, 2);
    //                 if (hasFocus) {
    //                     QColor softHighlight = option->palette.highlight().color();
    //                     softHighlight.setAlpha(40);
    //                     cachePainter.setPen(softHighlight);
    //                     cachePainter.drawRoundedRect(r.adjusted(1, 1, -2, -2), 1.7, 1.7);
    //                 }
    //                 cachePainter.restore();
    //             }

    //             // outline the up/down buttons
    //             cachePainter.setPen(outline);
    //             if (spinBox->direction == Qt::RightToLeft) {
    //                 cachePainter.drawLine(upRect.right(), upRect.top() - 1, upRect.right(), downRect.bottom() + 1);
    //             } else {
    //                 cachePainter.drawLine(upRect.left(), upRect.top() - 1, upRect.left(), downRect.bottom() + 1);
    //             }

    //             if (upIsActive && sunken) {
    //                 cachePainter.setPen(gradientStopColor.darker(130));
    //                 cachePainter.drawLine(downRect.left() + 1, downRect.top(), downRect.right(), downRect.top());
    //                 cachePainter.drawLine(upRect.left() + 1, upRect.top(), upRect.left() + 1, upRect.bottom());
    //                 cachePainter.drawLine(upRect.left() + 1, upRect.top() - 1, upRect.right(), upRect.top() - 1);
    //             }

    //             if (downIsActive && sunken) {
    //                 cachePainter.setPen(gradientStopColor.darker(130));
    //                 cachePainter.drawLine(downRect.left() + 1, downRect.top(), downRect.left() + 1, downRect.bottom() + 1);
    //                 cachePainter.drawLine(downRect.left() + 1, downRect.top(), downRect.right(), downRect.top());
    //                 cachePainter.setPen(gradientStopColor.darker(110));
    //                 cachePainter.drawLine(downRect.left() + 1, downRect.bottom() + 1, downRect.right(), downRect.bottom() + 1);
    //             }

    //             QColor disabledColor = mergedColors(arrowColor, option->palette.button().color());
    //             if (spinBox->buttonSymbols == QAbstractSpinBox::PlusMinus) {
    //                 int centerX = upRect.center().x();
    //                 int centerY = upRect.center().y();

    //                 // plus/minus
    //                 cachePainter.setPen((spinBox->stepEnabled & QAbstractSpinBox::StepUpEnabled) ? arrowColor : disabledColor);
    //                 cachePainter.drawLine(centerX - 1, centerY, centerX + 3, centerY);
    //                 cachePainter.drawLine(centerX + 1, centerY - 2, centerX + 1, centerY + 2);

    //                 centerX = downRect.center().x();
    //                 centerY = downRect.center().y();
    //                 cachePainter.setPen((spinBox->stepEnabled & QAbstractSpinBox::StepDownEnabled) ? arrowColor : disabledColor);
    //                 cachePainter.drawLine(centerX - 1, centerY, centerX + 3, centerY);

    //             } else if (spinBox->buttonSymbols == QAbstractSpinBox::UpDownArrows){
    //                 // arrows
    //                 qt_fusion_draw_arrow(Qt::UpArrow, &cachePainter, option, upRect.adjusted(0, 0, 0, 1),
    //                                      (spinBox->stepEnabled & QAbstractSpinBox::StepUpEnabled) ? arrowColor : disabledColor);
    //                 qt_fusion_draw_arrow(Qt::DownArrow, &cachePainter, option, downRect,
    //                                      (spinBox->stepEnabled & QAbstractSpinBox::StepDownEnabled) ? arrowColor : disabledColor);
    //             }

    //             cachePainter.end();
    //             QPixmapCache::insert(pixmapName, cache);
    //         }
    //         painter->drawPixmap(spinBox->rect.topLeft(), cache);
    //     }
    //     break;

    default:
        QCommonStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

QRect ModernStyle::subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r = QCommonStyle::subElementRect(sr, opt, w);
    switch (sr) {
    case SE_ProgressBarLabel:
    case SE_ProgressBarContents:
    case SE_ProgressBarGroove:
        return opt->rect;
    case SE_PushButtonFocusRect:
        r.adjust(0, 1, 0, -1);
        break;
    case SE_DockWidgetTitleBarText: {
        if (const QStyleOptionDockWidget *titlebar = qstyleoption_cast<const QStyleOptionDockWidget*>(opt)) {
            bool verticalTitleBar = titlebar->verticalTitleBar;
            if (verticalTitleBar) {
                r.adjust(0, 0, 0, -4);
            } else {
                if (opt->direction == Qt::LeftToRight)
                    r.adjust(4, 0, 0, 0);
                else
                    r.adjust(0, 0, -4, 0);
            }
        }
        break;
    }
    default:
        break;
    }
    return r;
}

QSize ModernStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const
{
    QSize newSize = QCommonStyle::sizeFromContents(type, option, size, widget);

    switch (type) {
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            if (!btn->text.isEmpty() && newSize.width() < 80)
                newSize.setWidth(80);
            if (!btn->icon.isNull() && btn->iconSize.height() > 16)
                newSize -= QSize(0, 2);
        }
        break;
    case CT_GroupBox:
        if (option) {
            int topMargin = qMax(pixelMetric(PM_ExclusiveIndicatorHeight), option->fontMetrics.height()) + groupBoxTopMargin;
            newSize += QSize(10, topMargin); // Add some space below the groupbox
        }
        break;
    case CT_RadioButton:
    case CT_CheckBox:
        newSize += QSize(0, 1);
        break;
    case CT_ToolButton:
        newSize += QSize(2, 2);
        break;
    case CT_SpinBox:
        newSize += QSize(0, -3);
        break;
    case CT_ComboBox:
        newSize += QSize(2, 4);
        break;
    case CT_LineEdit:
        newSize += QSize(0, 4);
        break;
    case CT_MenuBarItem:
        newSize += QSize(8, 5);
        break;
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            int w = newSize.width();
            int maxpmw = menuItem->maxIconWidth;
            int tabSpacing = 20;
            if (menuItem->text.contains(QLatin1Char('\t')))
                w += tabSpacing;
            else if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 2 * StyleHelper::dpiScaled(menuArrowHMargin, option);
            else if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
                QFontMetrics fm(menuItem->font);
                QFont fontBold = menuItem->font;
                fontBold.setBold(true);
                QFontMetrics fmBold(fontBold);
                w += fmBold.horizontalAdvance(menuItem->text) - fm.horizontalAdvance(menuItem->text);
            }
            const qreal dpi = StyleHelper::dpi(option);
            const int checkcol = qMax<int>(maxpmw, StyleHelper::dpiScaled(menuCheckMarkWidth, dpi)); // Windows always shows a check column
            w += checkcol;
            w += StyleHelper::dpiScaled(int(menuRightBorder) + 10, dpi);
            newSize.setWidth(w);
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                if (!menuItem->text.isEmpty()) {
                    newSize.setHeight(menuItem->fontMetrics.height());
                }
            }
            else if (!menuItem->icon.isNull()) {
                if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget)) {
                    newSize.setHeight(qMax(combo->iconSize().height() + 2, newSize.height()));
                }
            }
            newSize.setWidth(newSize.width() + int(StyleHelper::dpiScaled(12, dpi)));
            newSize.setWidth(qMax<int>(newSize.width(), int(StyleHelper::dpiScaled(120, dpi))));
        }
        break;
    case CT_SizeGrip:
        newSize += QSize(4, 4);
        break;
    case CT_TabBarTab: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
            newSize += QSize(tabbarMargin, tabbarMargin);
        }
    }
    case CT_MdiControls:
        newSize -= QSize(1, 0);
        break;
    default:
        break;
    }
    return newSize;
}

QStyle::SubControl ModernStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                                       const QPoint &pt, const QWidget *w) const
{
    return QCommonStyle::hitTestComplexControl(cc, opt, pt, w);
}

QRect ModernStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                   SubControl subControl, const QWidget *widget) const
{
    QRect rect = QCommonStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
            switch (subControl) {
            case SC_SliderHandle: {
                if (slider->orientation == Qt::Horizontal) {
                    rect.setHeight(proxy()->pixelMetric(PM_SliderThickness, option));
                    rect.setWidth(proxy()->pixelMetric(PM_SliderLength, option));
                    int centerY = slider->rect.center().y() - rect.height() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerY += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerY -= tickSize;
                    rect.moveTop(centerY);
                } else {
                    rect.setWidth(proxy()->pixelMetric(PM_SliderThickness, option));
                    rect.setHeight(proxy()->pixelMetric(PM_SliderLength, option));
                    int centerX = slider->rect.center().x() - rect.width() / 2;
                    if (slider->tickPosition & QSlider::TicksAbove)
                        centerX += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        centerX -= tickSize;
                    rect.moveLeft(centerX);
                }
            }
                break;
            case SC_SliderGroove: {
                QPoint grooveCenter = slider->rect.center();
                const int grooveThickness = StyleHelper::dpiScaled(7, option);
                if (slider->orientation == Qt::Horizontal) {
                    rect.setHeight(grooveThickness);
                    if (slider->tickPosition & QSlider::TicksAbove)
                        grooveCenter.ry() += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        grooveCenter.ry() -= tickSize;
                } else {
                    rect.setWidth(grooveThickness);
                    if (slider->tickPosition & QSlider::TicksAbove)
                        grooveCenter.rx() += tickSize;
                    if (slider->tickPosition & QSlider::TicksBelow)
                        grooveCenter.rx() -= tickSize;
                }
                rect.moveCenter(grooveCenter);
                break;
            }
            default:
                break;
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = spinbox->rect.height() / 2;
            int fw = spinbox->frame ? 3 : 0; // Is drawn with 3 pixels width in drawComplexControl, independently from PM_SpinBoxFrameWidth
            int y = fw;
            const int buttonWidth = StyleHelper::dpiScaled(14, option);
            int x, lx, rx;
            x = spinbox->rect.width() - y - buttonWidth + 2;
            lx = fw;
            rx = x - fw;
            switch (subControl) {
            case SC_SpinBoxUp:
                if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                rect = QRect(x, fw, buttonWidth, center - fw);
                break;
            case SC_SpinBoxDown:
                if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();

                rect = QRect(x, center, buttonWidth, spinbox->rect.bottom() - center - fw + 1);
                break;
            case SC_SpinBoxEditField:
                if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons) {
                    rect = QRect(lx, fw, spinbox->rect.width() - 2*fw, spinbox->rect.height() - 2*fw);
                } else {
                    rect = QRect(lx, fw, rx - qMax(fw - 1, 0), spinbox->rect.height() - 2*fw);
                }
                break;
            case SC_SpinBoxFrame:
                rect = spinbox->rect;
            default:
                break;
            }
            rect = visualRect(spinbox->direction, spinbox->rect, rect);
        }
        break;
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            rect = option->rect;
            if (subControl == SC_GroupBoxFrame)
                return rect.adjusted(0, 0, 0, 0);
            else if (subControl == SC_GroupBoxContents) {
                QRect frameRect = option->rect.adjusted(0, 0, 0, -groupBoxBottomMargin);
                int margin = 3;
                int leftMarginExtension = 0;
                int topMargin = qMax(pixelMetric(PM_ExclusiveIndicatorHeight), option->fontMetrics.height()) + groupBoxTopMargin;
                return frameRect.adjusted(leftMarginExtension + margin, margin + topMargin, -margin, -margin - groupBoxBottomMargin);
            }

            QSize textSize = option->fontMetrics.boundingRect(groupBox->text).size() + QSize(2, 2);
            int indicatorWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indicatorHeight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);

            const int width = textSize.width()
                + (option->subControls & QStyle::SC_GroupBoxCheckBox ? indicatorWidth + 5 : 0);

            rect = QRect();

            if (option->rect.width() > width) {
                switch (groupBox->textAlignment & Qt::AlignHorizontal_Mask) {
                case Qt::AlignHCenter:
                    rect.moveLeft((option->rect.width() - width) / 2);
                    break;
                case Qt::AlignRight:
                    rect.moveLeft(option->rect.width() - width);
                    break;
                }
            }

            if (subControl == SC_GroupBoxCheckBox) {
                rect.setWidth(indicatorWidth);
                rect.setHeight(indicatorHeight);
                rect.moveTop(textSize.height() > indicatorHeight ? (textSize.height() - indicatorHeight) / 2 : 0);
                rect.translate(1, 0);
            } else if (subControl == SC_GroupBoxLabel) {
                rect.setSize(textSize);
                rect.moveTop(1);
                if (option->subControls & QStyle::SC_GroupBoxCheckBox)
                    rect.translate(indicatorWidth + 5, 0);
            }
            return visualRect(option->direction, option->rect, rect);
        }

        return rect;

    case CC_ComboBox:
        switch (subControl) {
        case SC_ComboBoxArrow: {
            const qreal dpi = StyleHelper::dpi(option);
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(rect.right() - int(StyleHelper::dpiScaled(18, dpi)), rect.top() - 2,
                         int(StyleHelper::dpiScaled(19, dpi)), rect.height() + 4);
            rect = visualRect(option->direction, option->rect, rect);
        }
            break;
        case SC_ComboBoxEditField: {
            int frameWidth = 2;
            rect = visualRect(option->direction, option->rect, rect);
            rect.setRect(option->rect.left() + frameWidth, option->rect.top() + frameWidth,
                         option->rect.width() - int(StyleHelper::dpiScaled(19, option)) - 2 * frameWidth,
                         option->rect.height() - 2 * frameWidth);
            if (const QStyleOptionComboBox *box = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
                if (!box->editable) {
                    rect.adjust(2, 0, 0, 0);
                    if (box->state & (State_Sunken | State_On))
                        rect.translate(1, 1);
                }
            }
            rect = visualRect(option->direction, option->rect, rect);
            break;
        }
        default:
            break;
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            SubControl sc = subControl;
            QRect &ret = rect;
            const int indent = 3;
            const int controlTopMargin = 3;
            const int controlBottomMargin = 3;
            const int controlWidthMargin = 2;
            const int controlHeight = tb->rect.height() - controlTopMargin - controlBottomMargin ;
            const int delta = controlHeight + controlWidthMargin;
            int offset = 0;

            bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
            bool isMaximized = tb->titleBarState & Qt::WindowMaximized;

            switch (sc) {
            case SC_TitleBarLabel:
                if (tb->titleBarFlags & (Qt::WindowTitleHint | Qt::WindowSystemMenuHint)) {
                    ret = tb->rect;
                    if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                        ret.adjust(delta, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowShadeButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                    if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                        ret.adjust(0, 0, -delta, 0);
                }
                break;
            case SC_TitleBarContextHelpButton:
                if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                    offset += delta;
                Q_FALLTHROUGH();
            case SC_TitleBarMinButton:
                if (!isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarMinButton)
                    break;
                Q_FALLTHROUGH();
            case SC_TitleBarNormalButton:
                if (isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                    offset += delta;
                else if (isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarNormalButton)
                    break;
                Q_FALLTHROUGH();
            case SC_TitleBarMaxButton:
                if (!isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarMaxButton)
                    break;
                Q_FALLTHROUGH();
            case SC_TitleBarShadeButton:
                if (!isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarShadeButton)
                    break;
                Q_FALLTHROUGH();
            case SC_TitleBarUnshadeButton:
                if (isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                    offset += delta;
                else if (sc == SC_TitleBarUnshadeButton)
                    break;
                Q_FALLTHROUGH();
            case SC_TitleBarCloseButton:
                if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                    offset += delta;
                else if (sc == SC_TitleBarCloseButton)
                    break;
                ret.setRect(tb->rect.right() - indent - offset, tb->rect.top() + controlTopMargin,
                            controlHeight, controlHeight);
                break;
            case SC_TitleBarSysMenu:
                if (tb->titleBarFlags & Qt::WindowSystemMenuHint) {
                    ret.setRect(tb->rect.left() + controlWidthMargin + indent, tb->rect.top() + controlTopMargin,
                                controlHeight, controlHeight);
                }
                break;
            default:
                break;
            }
            ret = visualRect(tb->direction, tb->rect, ret);
        }
        break;
    default:
        break;
    }

    return rect;
}

QPixmap ModernStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                          const QStyleOption *opt) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

int ModernStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                            QStyleHintReturn *returnData) const
{
    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
    }

    switch (hint) {
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_MenuBar_AltKeyNavigation:
    case SH_ComboBox_ListMouseTracking:
    case SH_Slider_StopMouseOverSlider:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_EtchDisabledText:
    case SH_TitleBar_AutoRaise:
    case SH_TitleBar_NoBorder:
    case SH_ItemView_ShowDecorationSelected:
    case SH_ItemView_ArrowKeysNavigateIntoChildren:
    case SH_ItemView_ChangeHighlightOnFocus:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
    case SH_Menu_SupportsSections:
        return 1;
    case SH_ComboBox_UseNativePopup:
        return 1;
    case SH_ToolBox_SelectedPageTitleBold:
    case SH_ScrollView_FrameOnlyAroundContents:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_MessageBox_CenterButtons:
    case SH_RubberBand_Mask:
        return 0;

    case SH_ScrollBar_Transient:
        return 1;

    case SH_ComboBox_Popup:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            return !cmb->editable;
        return 0;

    case SH_Table_GridLineColor:
        return option ? option->palette.window().color().darker(120).rgba() : 0;

    case SH_MessageBox_TextInteractionFlags:
        return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
    case SH_Menu_SubMenuPopupDelay:
        return 225; // default from GtkMenu

    case SH_WindowFrame_Mask:
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData)) {
            //left rounded corner
            mask->region = option->rect;
            mask->region -= QRect(option->rect.left(), option->rect.top(), 5, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 1, 3, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 2, 2, 1);
            mask->region -= QRect(option->rect.left(), option->rect.top() + 3, 1, 2);

            //right rounded corner
            mask->region -= QRect(option->rect.right() - 4, option->rect.top(), 5, 1);
            mask->region -= QRect(option->rect.right() - 2, option->rect.top() + 1, 3, 1);
            mask->region -= QRect(option->rect.right() - 1, option->rect.top() + 2, 2, 1);
            mask->region -= QRect(option->rect.right() , option->rect.top() + 3, 1, 2);
            return 1;
        }
    default:
        break;
    }
    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

void ModernStyle::drawItemPixmap(QPainter *painter, const QRect &rect,
                                  int alignment, const QPixmap &pixmap) const
{
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void ModernStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                                bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    QPen savedPen = painter->pen();
    if (textRole != QPalette::NoRole) {
        painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
    }
    if (!enabled) {
        QPen pen = painter->pen();
        painter->setPen(pen);
    }
    painter->drawText(rect, alignment, text);
    painter->setPen(savedPen);
}

void ModernStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
    if (false
            || qobject_cast<QAbstractButton*>(widget)
            || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QProgressBar *>(widget)
            || qobject_cast<QScrollBar *>(widget)
            || qobject_cast<QSplitterHandle *>(widget)
            || qobject_cast<QAbstractSlider *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget)
            || (widget->inherits("QDockSeparator"))
            || (widget->inherits("QDockWidgetSeparator"))
            ) {
        widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

    if (widget->inherits("QMenu")) {
        m_blurHelper->registerWidget(widget);
    }

    const auto blurBehindProperty = widget->property(s_blurBehindPropertyName.constData());
    if (blurBehindProperty.toBool()) {
        m_blurHelper->registerWidget(widget);
    }
}

void ModernStyle::polish(QApplication *app)
{
    QCommonStyle::polish(app);

    app->setPalette(standardPalette());
}

void ModernStyle::polish(QPalette &pal)
{
    QCommonStyle::polish(pal);
}

void ModernStyle::unpolish(QWidget *widget)
{
    QCommonStyle::unpolish(widget);
    if (false
            || qobject_cast<QAbstractButton*>(widget)
            || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QProgressBar *>(widget)
            || qobject_cast<QScrollBar *>(widget)
            || qobject_cast<QSplitterHandle *>(widget)
            || qobject_cast<QAbstractSlider *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget)
            || (widget->inherits("QDockSeparator"))
            || (widget->inherits("QDockWidgetSeparator"))
            ) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if (widget->inherits("QMenu")) {
        m_blurHelper->unregisterWidget(widget);
    }
    const auto blurBehindProperty = widget->property(s_blurBehindPropertyName.constData());
    if (blurBehindProperty.toBool()) {
        m_blurHelper->unregisterWidget(widget);
    }
}

void ModernStyle::unpolish(QApplication *app)
{
    QCommonStyle::unpolish(app);
}
