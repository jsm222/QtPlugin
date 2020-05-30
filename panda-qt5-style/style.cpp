#include "style.h"
#include "styleanimation.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QWidget>
#include <QWindow>
#include <QMenu>
#include <QAbstractItemView>
#include <QFontMetrics>
#include <QPainter>

static const QByteArray s_blurBehindPropertyName = QByteArrayLiteral("ENABLE_BLUR_BEHIND_HINT");

static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : 0;
}

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = nullptr)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;
    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *toolbutton;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
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

static QColor outline(const QPalette &pal)
{
    if (pal.window().style() == Qt::TexturePattern)
        return QColor(0, 0, 0, 160);

    return pal.window().color().darker(140);
}

static QColor highlightedOutline(const QPalette &pal)
{
    QColor highlightedOutline = pal.color(QPalette::Highlight).darker(125);
    if (highlightedOutline.value() > 160)
        highlightedOutline.setHsl(highlightedOutline.hue(), highlightedOutline.saturation(), 160);
    return highlightedOutline;
}

Style::Style()
    : QProxyStyle("fusion"),
      m_blurHelper(new BlurHelper(this))
{
    // QStyleFactory::create("panda")
    // Code reference qfusionstyle.cpp
}

void Style::polish(QWidget *w)
{
    QProxyStyle::polish(w);

    if (w->inherits("QPushButton") || w->inherits("QCheckBox")
     || w->inherits("QComboBox") || w->inherits("QRadioButton")
     || w->inherits("QScrollBar") || w->inherits("QToolButton")
     || w->inherits("QAbstractSpinBox") || w->inherits("QAbstractSpinBox")
     || w->inherits("QTabBar"))
        w->setAttribute(Qt::WA_Hover, true);

    if (w->inherits("QScrollBar"))
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);

    // transparent tooltips
    if (w->inherits("QTipLabel")) {
        w->setAttribute(Qt::WA_TranslucentBackground, true);
    }

    if (qobject_cast<QMenu *>(w)) {
        //w->setAttribute(Qt::WA_TranslucentBackground, true);
        m_blurHelper->registerWidget(w);
    }

    const auto blurBehindProperty = w->property(s_blurBehindPropertyName.constData());

    if (blurBehindProperty.toBool()) {
        m_blurHelper->registerWidget(w);
    }
}

void Style::unpolish(QWidget *w)
{
    QProxyStyle::unpolish(w);

    if (w->inherits("QPushButton") || w->inherits("QCheckBox")
     || w->inherits("QComboBox") || w->inherits("QRadioButton")
     || w->inherits("QScrollBar") || w->inherits("QToolButton")
     || w->inherits("QAbstractSpinBox") || w->inherits("QAbstractSpinBox")
     || w->inherits("QTabBar"))
        w->setAttribute(Qt::WA_Hover, false);

    if (w->inherits("QTipLabel") || w->inherits("QMenu")) {
        //w->setAttribute(Qt::WA_TranslucentBackground, false);
        m_blurHelper->unregisterWidget(w);
    }

    const auto blurBehindProperty = w->property(s_blurBehindPropertyName.constData());

    if (blurBehindProperty.toBool()) {
        m_blurHelper->unregisterWidget(w);
    }
}

void Style::polish(QApplication *app)
{
    QFont font("Noto Mono");
    font.setPixelSize(15);
    app->setFont(font);
}

void Style::polish(QPalette &palette)
{
    QColor windowBg(255, 255, 255);
    QColor fontColor(23, 23, 23);
    QColor disableColor(150, 150, 150);
    QColor themeColor(84, 156, 255);

    palette.setBrush(QPalette::Window, windowBg);
    palette.setBrush(QPalette::WindowText, fontColor);
    palette.setBrush(QPalette::Text, fontColor);

    palette.setBrush(QPalette::Disabled, QPalette::WindowText, disableColor);
    palette.setBrush(QPalette::Disabled, QPalette::Text, disableColor);
    palette.setBrush(QPalette::Disabled, QPalette::BrightText, disableColor);
    // palette.setBrush(QPalette::PlaceholderText, disableColor);

    palette.setBrush(QPalette::Highlight, themeColor);
    palette.setBrush(QPalette::HighlightedText, Qt::white);
    palette.setBrush(QPalette::Active, QPalette::Highlight, themeColor);
}

void Style::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QRect rect = option->rect;
    int state = option->state;

    switch (element) {
    case PE_Frame: {
        break;
    }

    case PE_IndicatorItemViewItemDrop: {
        const qreal radius = Frame_FrameRadius;
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

    case PE_FrameTabBarBase: {
        // 不繪畫tabbar邊框
        break;
    }

    case PE_FrameTabWidget: {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(option->rect);
        break;
    }

    case PE_PanelMenu:
    case PE_FrameMenu: {
        return drawMenu(option, painter, widget);
        break;
    }

    case PE_PanelLineEdit: {
        painter->save();
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->fillRect(panel->rect, Qt::transparent);
        }
        painter->restore();
    }

    case PE_FrameLineEdit: {
        QRect r = rect;
        bool hasFocus = option->state & State_HasFocus;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        //  ### highdpi painter bug.
        painter->translate(0.5, 0.5);
        painter->fillRect(r, Qt::transparent);

        const QColor outline = QColor(195, 195, 195, 100);
        const QColor highlightedOutline = option->palette.color(QPalette::Highlight);

        // Draw Outline
        painter->setPen(QPen(hasFocus ? highlightedOutline : outline));
        painter->setBrush(QColor(255, 255, 255, 100));

        const qreal radius = r.height() * 0.2;
        painter->drawRoundedRect(r.adjusted(0, 0, -radius / 2, - radius / 2), radius, radius);

        // Draw inner shadow
        // painter->setPen(QColor(242, 242, 242));
        // painter->drawLine(QPoint(r.left() + 2, r.top() + 1), QPoint(r.right() - 2, r.top() + 1));

        painter->restore();
        break;
    }

    case PE_IndicatorItemViewItemCheck: {
        QStyleOptionButton button;
        button.QStyleOption::operator=(*option);
        button.state &= ~State_MouseOver;
        proxy()->drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
        break;
    }

    case PE_IndicatorHeaderArrow: {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(Qt::NoBrush);
            if(option->state & State_Enabled){
                painter->setPen(QPen(option->palette.foreground().color(), 1.1));
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
        break;
    }

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

                if (isIconView)
                    painter->drawRoundedRect(option->rect, Frame_FrameRadius, Frame_FrameRadius);
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

            QColor outlineColor = outline(option->palette);
            painter->setBrush((state & State_Sunken) ? QBrush(pressedColor) : gradient);
            painter->setPen(QPen(outlineColor.lighter(110)));
            if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange)
                painter->setPen(option->palette.color(QPalette::Highlight));
            painter->drawRoundedRect(rect, 3, 3);

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
                painter->setPen(checkMarkColor);
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

    case PE_PanelButtonCommand: {
        return drawPanelButtonCommandPrimitive(option, painter, widget);
    }

    case PE_PanelButtonTool: {
        return drawPanelButtonToolPrimitive(option, painter, widget);
    }

    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void Style::drawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (drawTabBar(painter, tab, widget))
                return;
        }
        break;

    case CE_PushButtonBevel: {
        return drawPanelButtonCommandPrimitive(opt, painter, widget);
    }

    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (drawTabBarLabel(painter, tab, widget))
                return;
        }
        break;

    case CE_MenuItem: {
        return drawMenuItem(opt, painter, widget);
    }

    case CE_ScrollBarSlider: {
        const QStyleOptionSlider *sliderOption(qstyleoption_cast<const QStyleOptionSlider *>(opt));
        if (!sliderOption)
            break;

        const State &state(opt->state);
        bool horizontal(state & State_Horizontal);

        // copy rect and palette
        const QRect &rect(horizontal ? opt->rect.adjusted(-1, 4, 0, -4) : opt->rect.adjusted(4, -1, -4, 0));
        const QPalette &palette(opt->palette);

        // define handle rect
        QRect handleRect;

        bool enabled(state & State_Enabled);
        bool mouseOver((state & State_Active) && enabled && (state & State_MouseOver));
        bool sunken(enabled && (state & (State_On | State_Sunken)));
        qreal opacity;
        if (mouseOver)
            opacity = 0.7;
        else
            opacity = 0.2;

        if (horizontal) {
            handleRect = rect.adjusted(0, 6, 0, 2);
            handleRect.adjust(0, -6.0 * opacity, 0, -2.0 * opacity);
        } else {
            handleRect = rect.adjusted(6, 0, 2, 0);
            handleRect.adjust(-6.0 * opacity, 0, -2.0 * opacity, 0);
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        qreal metric(handleRect.width() < handleRect.height() ? handleRect.width() : handleRect.height());
        qreal radius(0.5 * metric);
        painter->setPen(Qt::NoPen);
        painter->setOpacity(opacity);
        painter->setBrush(opt->palette.windowText());
        painter->drawRoundedRect(handleRect, radius, radius);
        painter->restore();
        break;
    }

    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            painter->save();
            painter->setClipRect(opt->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

            // draw the background
            proxy()->drawPrimitive(PE_PanelItemViewItem, opt, painter, widget);

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

            // 不绘画 focus 状态
            painter->restore();
        }
    break;

    // table header style.
    case CE_HeaderSection: {
        const auto headerOption(qstyleoption_cast<const QStyleOptionHeader*>(opt));
        if (!headerOption) return;
        const bool horizontal(headerOption->orientation == Qt::Horizontal);
        const bool isLast(headerOption->position == QStyleOptionHeader::End);

        // draw background
        painter->fillRect(opt->rect, QBrush(opt->palette.alternateBase().color()));

        // draw line
        QColor lineColor(opt->palette.alternateBase().color().darker(110));
        painter->setPen(lineColor);
        if (horizontal) {
            if (!isLast) {
                QPoint unit(0, opt->rect.height() / 5);
                painter->drawLine(opt->rect.topRight() + unit, opt->rect.bottomRight() - unit);
            }
            painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
        } else {
            if (!isLast) {
                painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
            }
            painter->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
        }
        break;
    }

    case CE_ToolButtonLabel: {
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                painter->setFont(toolbutton->font);

                if (toolbutton->state & State_On) {
                    painter->setPen(opt->palette.color(QPalette::HighlightedText));
                }

                if (!(toolbutton->state & State_Enabled))
                    painter->setPen(opt->palette.color(QPalette::Disabled, QPalette::ButtonText));

                proxy()->drawItemText(painter, rect, alignment, toolbutton->palette,
                                      opt->state & State_Enabled, toolbutton->text);
            } else {
                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;
                if (!toolbutton->icon.isNull()) {
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->icon.pixmap(qt_getWindow(widget), toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                                 mode, state);
                    pmSize = pm.size() / pm.devicePixelRatio();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    painter->setFont(toolbutton->font);
                    QRect pr = rect,
                        tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(0, pr.height() - 1, 0, -1);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(pr.width(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, QStyle::visualRect(opt->direction, rect, pr), Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    const QString text = toolButtonElideText(toolbutton, tr, alignment);
                    if (toolbutton->state & State_On) {
                        painter->setPen(opt->palette.color(QPalette::HighlightedText));
                    }

                    if (!(toolbutton->state & State_Enabled))
                        painter->setPen(opt->palette.color(QPalette::Disabled, QPalette::ButtonText));

                    proxy()->drawItemText(painter, QStyle::visualRect(opt->direction, rect, tr), alignment, toolbutton->palette,
                                          toolbutton->state & State_Enabled, text);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(proxy(), toolbutton, rect, painter, widget);
                    } else {
                        proxy()->drawItemPixmap(painter, rect, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        break;
    }

    default:
        QProxyStyle::drawControl(element, opt, painter, widget);
        break;
    }
}

QRect Style::subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *option, QStyle::SubControl sc, const QWidget *widget) const
{
    switch (cc) {
    case CC_ScrollBar: {
        return scrollBarSubControlRect(option, sc, widget);
    }

    default:
        return QProxyStyle::subControlRect(cc, option, sc, widget);
    }
}

QRect Style::subElementRect(QStyle::SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    switch (r) {
    default:
        break;
    }

    return QProxyStyle::subElementRect(r, opt, widget);
}

void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                      QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        bool enabled(option->state & State_Enabled);
        bool mouseOver((option->state & State_Active) && option->state & State_MouseOver);
        // render full groove directly, rather than using the addPage and subPage control element methods
        if (mouseOver && option->subControls & SC_ScrollBarGroove) {
            // retrieve groove rectangle
            // QRect grooveRect(subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));
            // const State &state(option->state);
            // bool horizontal(state & State_Horizontal);
            // if (horizontal)
            //     grooveRect = centerRect(grooveRect, grooveRect.width(), Metrics::ScrollBar_SliderWidth);
            // else
            //     grooveRect = centerRect(grooveRect, Metrics::ScrollBar_SliderWidth, grooveRect.height());
            // render
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

    // case CC_ToolButton: {
    //     if (const QStyleOptionToolButton *buttonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
    //         QRect buttonRect = proxy()->subControlRect(control, buttonOption, SC_ToolButton, widget);
    //         painter->save();
    //         painter->setBrush(Qt::red);
    //         painter->drawRect(buttonRect);
    //         painter->restore();
    //     }
    //     break;
    // }

    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

static void setAttribute(const QWidget *w)
{
    if (!w || w->testAttribute(Qt::WA_WState_Created))
        return;

    if (auto menu = qobject_cast<const QMenu *>(w)) {
        const_cast<QWidget *>(w)->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (w->inherits("QComboBoxPrivateContainer")) {
        const_cast<QWidget *>(w)->setAttribute(Qt::WA_TranslucentBackground);
    }
}

int Style::styleHint(QStyle::StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    setAttribute(w);

    switch (sh) {
    case SH_ComboBox_Popup:
        return false;
    case SH_ScrollBar_Transient:
        return true;
    default:
        break;
    }

    return QProxyStyle::styleHint(sh, opt, w, shret);
}

int Style::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_MenuHMargin: return 9;
    case PM_MenuVMargin: return 19;
    case PM_SubMenuOverlap: return -2;
    case PM_ButtonMargin: return 10;
    case PM_HeaderMargin: return 10;

    // scrollbars
    case PM_ScrollBarExtent:
        return Metrics::ScrollBar_Extend;
    case PM_ScrollBarSliderMin:
        return Metrics::ScrollBar_MinSliderHeight;

    case PM_SplitterWidth:
        return Metrics::Splitter_SplitterWidth;
    case PM_DockWidgetSeparatorExtent:
        return Metrics::Splitter_SplitterWidth;

    default:
        break;
    }

    return QProxyStyle::pixelMetric(metric, option, widget);
}
