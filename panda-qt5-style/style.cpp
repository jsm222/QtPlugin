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
    QFont font("Ubuntu");
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
    palette.setBrush(QPalette::Disabled, QPalette::BrightText,disableColor);
    palette.setBrush(QPalette::PlaceholderText, disableColor);

    palette.setBrush(QPalette::Highlight, themeColor);
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
            bool isIconView = (vopt->decorationPosition & QStyleOptionViewItem::Top);

            if (vopt->state & QStyle::State_Selected) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(Qt::transparent);
                painter->setBrush(vopt->palette.brush(cg, QPalette::Highlight));

                if (isIconView)
                    painter->drawRoundedRect(option->rect, 6, 6);
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
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(242, 242, 242));

        if (widget->property("ENABLE_TRANSPARENT").toBool())
            painter->setBrush(Qt::transparent);

        if (option->state & State_MouseOver) {
            // press
            if (option->state & State_Sunken) {
                painter->setBrush(QColor(204, 204, 204));
            } else {
            // hover
                painter->setBrush(QColor(224, 224, 224));
            }
        }
        painter->drawRoundedRect(option->rect,6, 6);
        painter->restore();
        break;
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

    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (drawTabBarLabel(painter, tab, widget))
                return;
        }
        break;

    case CE_MenuItem: {
        return drawMenuItem(opt, painter, widget);
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
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillRect(opt->rect, opt->palette.alternateBase().color());
        painter->restore();
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
        auto rect = QProxyStyle::subControlRect(cc, option, sc, widget);
        if (sc == SC_ScrollBarSlider) {
            rect.adjust(1, 1, -1, -1);
            if (option->state.testFlag(QStyle::State_Horizontal)) {
                rect.adjust(1, 0, -1, 0);
            } else {
                rect.adjust(0, 1, 0, -1);
            }
            return rect;
        }
        return rect;
    }
    }

    return QProxyStyle::subControlRect(cc, option, sc, widget);
}

void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                      QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

void setWidgetAttribute(const QWidget *w)
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
    setWidgetAttribute(w);

    switch (sh) {
    case  SH_ComboBox_Popup:
        return false;
    default:
        break;
    }

    return QProxyStyle::styleHint(sh, opt, w, shret);
}

int Style::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent: return 12;
    case PM_ScrollBarSliderMin: return 40;
    case PM_MenuHMargin: return 9;
    case PM_MenuVMargin: return 19;
    case PM_SubMenuOverlap: return -2;
    case PM_ButtonMargin: return 10;
    case PM_HeaderMargin: return 10;
    default:
        break;
    }

    return QProxyStyle::pixelMetric(metric, option, widget);
}
