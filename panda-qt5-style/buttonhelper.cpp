#include "style.h"
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>

void Style::drawPanelButtonCommandPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    // cast option and check
    const auto buttonOption(qstyleoption_cast<const QStyleOptionButton* >(option));
    if (!buttonOption) return;

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

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (flat) {
        const auto &palette(option->palette);
        // render toolbutton frame
        painter->setPen(Qt::NoPen);

        if (sunken) {
            painter->setBrush(Qt::red);
        } else {
            painter->setBrush(Qt::green);
        }

        painter->drawRoundedRect(rect, radius, radius);
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(242, 242, 242));
        const QRectF baseRect( rect.adjusted( 1, 1, -1, -1 ) );

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
            painter->setPen(option->palette.highlight().color());
        }

        painter->drawRoundedRect(baseRect, radius, radius);
    }
}
