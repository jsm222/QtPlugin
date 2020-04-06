#include "style.h"
#include <QPainter>
#include <QPainterPath>
#include <QComboBox>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

void Style::drawMenu(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(option->rect.adjusted(9, 9, -9, -9), 10, 10);

    QPixmap pixmap(option->rect.size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    QImage img = pixmap.toImage();
    qt_blurImage(img, 8, false, false);

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
        path.addRoundedRect(option->rect.adjusted(9, 9, -9, -9), 10, 10);
    } else {
        path.addRegion(region);
    }

    painter->drawPath(path);
    painter->restore();
}

void Style::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    // Draws one item in a popup menu.
    if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
        QColor highlightOutline = option->palette.highlight().color();
        QColor highlight = option->palette.highlight().color();
        if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            int w = 0;
            // const int margin = int(QStyleHelper::dpiScaled(5, option));
            const int margin = 5;
            if (!menuItem->text.isEmpty()) {
                painter->setFont(menuItem->font);
                proxy()->drawItemText(painter, menuItem->rect.adjusted(margin, 0, -margin, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                    menuItem->palette, menuItem->state & State_Enabled, menuItem->text,
                                    QPalette::Text);
                w = menuItem->fontMetrics.horizontalAdvance(menuItem->text) + margin;
            }
            painter->setPen(option->palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->setPen(QColor(0, 0, 0, 30));
            bool reverse = menuItem->direction == Qt::RightToLeft;
            painter->drawLine(menuItem->rect.left() + margin + (reverse ? 0 : w), menuItem->rect.center().y(),
                            menuItem->rect.right() - margin - (reverse ? w : 0), menuItem->rect.center().y());
            painter->restore();
            return;
        }
        bool selected = menuItem->state & State_Selected && menuItem->state & State_Enabled;
        if (selected) {
            QRect r = option->rect;
            painter->fillRect(r, highlight);
            painter->setPen(QPen(highlightOutline));
            painter->drawRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5));
        }
        bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
        bool checked = menuItem->checked;
        bool sunken = menuItem->state & State_Sunken;
        bool enabled = menuItem->state & State_Enabled;

        bool ignoreCheckMark = false;
        const int windowsItemHMargin = 5;
        const int windowsItemFrame = 0;
        const int checkColHOffset = windowsItemHMargin + windowsItemFrame - 1;
        int checkcol = qMax<int>(menuItem->rect.height() * 0.79,   // dpiScaled(21, option)
                                qMax<int>(menuItem->maxIconWidth, 21)); // icon checkbox's highlight column width
        if (qobject_cast<const QComboBox *>(widget) ||
            (option->styleObject && option->styleObject->property("_q_isComboBoxPopupItem").toBool()))
            ignoreCheckMark = true; //ignore the checkmarks provided by the QComboMenuDelegate

        if (!ignoreCheckMark) {
            // Check, using qreal and QRectF to avoid error accumulation
            // const qreal boxMargin = dpiScaled(3.5, option);
            const qreal boxMargin = 3.5;
            const qreal boxWidth = checkcol - 2 * boxMargin;
            QRectF checkRectF(option->rect.left() + boxMargin + checkColHOffset, option->rect.center().y() - boxWidth/2 + 1, boxWidth, boxWidth);
            QRect checkRect = checkRectF.toRect();
            checkRect.setWidth(checkRect.height()); // avoid .toRect() round error results in non-perfect square
            checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
            if (checkable) {
                if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
                    // Radio button
                    if (checked || sunken) {
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
                        if (checked)
                            box.state |= State_On;
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

            if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget))
                iconSize = combo->iconSize();
            if (checked)
                pixmap = menuItem->icon.pixmap(iconSize, mode, QIcon::On);
            else
                pixmap = menuItem->icon.pixmap(iconSize, mode);

            const int pixw = pixmap.width() / pixmap.devicePixelRatio();
            const int pixh = pixmap.height() / pixmap.devicePixelRatio();

            QRect pmr(0, 0, pixw, pixh);
            pmr.moveCenter(vCheckRect.center());
            painter->setPen(menuItem->palette.text().color());
            if (!ignoreCheckMark && checkable && checked) {
                QStyleOption opt = *option;
                if (act) {
                    QColor activeColor = option->palette.background().color();
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
        int windowsItemVMargin = 5;
        int windowsRightBorder = 5;

        QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
        QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
        QStringRef s(&menuitem->text);
        if (!s.isEmpty()) {                     // draw text
            p->save();
            int t = s.indexOf(QLatin1Char('\t'));
            int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!QProxyStyle::styleHint(SH_UnderlineShortcut, menuitem, widget))
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
}