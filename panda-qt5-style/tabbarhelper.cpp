#include "style.h"
#include <QPainter>

bool Style::drawTabBar(QPainter *painter,  const QStyleOptionTab *tab, const QWidget *widget) const
{
    if (!qobject_cast<const QTabBar *>(widget))
        return false;

    painter->fillRect(tab->rect, Qt::transparent);
    painter->save();

    bool isTriangularMode = false;
    bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                       && (tab->shape == QTabBar::RoundedNorth
                           || tab->shape == QTabBar::RoundedSouth));
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;
    bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                    || (rtlHorTabs
                        && tab->position == QStyleOptionTab::Beginning));
    bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
    int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, tab, widget);
    QRect rect = tab->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

    QRect r2(rect);
    int x1 = r2.left();
    int x2 = r2.right();
    int y1 = r2.top();
    int y2 = r2.bottom();

    QTransform rotMatrix;
    bool flip = false;
    painter->setPen(QColor(0, 0, 0, 100));

    switch (tab->shape) {
    case QTabBar::TriangularNorth:
        rect.adjust(0, 0, 0, -tabOverlap);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularSouth:
        rect.adjust(0, tabOverlap, 0, 0);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularEast:
        rect.adjust(tabOverlap, 0, 0, 0);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularWest:
        rect.adjust(0, 0, -tabOverlap, 0);
        isTriangularMode = true;
        break;
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

    QColor lineColor = !isTriangularMode || selected ? Qt::transparent : QColor(Qt::red);
    QColor tabFrameColor = selected ? Qt::transparent : Qt::transparent;

    // if (!(tab->features & QStyleOptionTab::HasFrame))
    //     tabFrameColor = QColor(0, 0, 0, 100);

    if (!isTriangularMode) {
        tabFrameColor = selected ? tab->palette.color(QPalette::Highlight) : Qt::transparent;
    }

    QPen outlinePen(lineColor, proxy()->pixelMetric(PM_DefaultFrameWidth, tab, widget));
    QRect drawRect = rect;
    painter->setPen(outlinePen);
    painter->setBrush(tabFrameColor);
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!isTriangularMode) {
        int radius = 6;
        int border = 2;
        painter->drawRoundedRect(drawRect.adjusted(border, border, -border, -border), radius, radius);
    } else {
        painter->drawRect(drawRect);
    }

    painter->restore();

    return true;
}

bool Style::drawTabBarLabel(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const
{
    const QTabBar *m_tabbar = static_cast<const QTabBar *>(widget);

    if (!m_tabbar)
        return false;

    bool isTriangularMode = false;
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;

    switch (tab->shape) {
    case QTabBar::TriangularNorth:
    case QTabBar::TriangularSouth:
    case QTabBar::TriangularEast:
    case QTabBar::TriangularWest:
        isTriangularMode = true;
        break;
    default:
        break;
    }

    QRect tr = proxy()->subElementRect(SE_TabBarTabText, tab, widget);
    QRect textRect = tab->fontMetrics.boundingRect(tr, Qt::AlignCenter | Qt::TextShowMnemonic, tab->text);
    int close_button_width = proxy()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, tab, widget);
    qreal stop = qreal(tr.right() - close_button_width - textRect.x() - 5) / textRect.width();

    if (selected) {
        painter->setPen(QColor(255, 255, 255));
    } else {
        painter->setPen(QColor(0, 0, 0, 180));
    }

    QFont font = painter->font();
    font.setBold(false);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextShowMnemonic, tab->text);

    // QProxyStyle::drawControl(CE_TabBarTabLabel, tab, painter, widget);

    return true;
}