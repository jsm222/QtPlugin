#include "style.h"
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>

void Style::drawPanelButtonCommandPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
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
        const qreal radius(Style::Frame_FrameRadius);

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
}

void Style::drawPanelButtonToolPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto &rect(option->rect);
    const State &state(option->state);
    const bool sunken(state & (State_On | State_Sunken));
    const qreal radius(Style::Frame_FrameRadius);

    if (state & State_On) {
        painter->setBrush(option->palette.highlight().color());
    } else {
        painter->setBrush(Qt::transparent);
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawRoundedRect(option->rect, radius, radius);
    painter->restore();
}
