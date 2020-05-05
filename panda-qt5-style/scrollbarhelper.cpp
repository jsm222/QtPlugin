#include "style.h"

QRect Style::scrollBarInternalSubControlRect(const QStyleOptionComplex *option, SubControl subControl) const
{
    const QRect &rect = option->rect;
    const State &state(option->state);
    bool horizontal(state & State_Horizontal);

    switch (subControl) {
    case SC_ScrollBarSubLine: {
        int majorSize = Metrics::ScrollBar_SingleButtonHeight;
        if (horizontal)
            return visualRect(option, QRect(rect.left(), rect.top(), majorSize, rect.height()));
        else
            return visualRect(option, QRect(rect.left(), rect.top(), rect.width(), majorSize));
    }

    case SC_ScrollBarAddLine: {
        int majorSize = Metrics::ScrollBar_SingleButtonHeight;
        if (horizontal)
            return visualRect(option, QRect(rect.right() - majorSize + 1, rect.top(), majorSize, rect.height()));
        else
            return visualRect(option, QRect(rect.left(), rect.bottom() - majorSize + 1, rect.width(), majorSize));
    }

    default:
        return QRect();
    }
}

QRect Style::scrollBarSubControlRect(const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    // cast option and check
    const QStyleOptionSlider *sliderOption(qstyleoption_cast<const QStyleOptionSlider *>(option));
    if (!sliderOption)
        return QProxyStyle::subControlRect(CC_ScrollBar, option, subControl, widget);

    // get relevant state
    const State &state(option->state);
    bool horizontal(state & State_Horizontal);

    switch (subControl) {
    case SC_ScrollBarSubLine:
    case SC_ScrollBarAddLine:
        return QRect();

    case SC_ScrollBarGroove: {
        QRect topRect = visualRect(option, scrollBarInternalSubControlRect(option, SC_ScrollBarSubLine));
        QRect bottomRect = visualRect(option, scrollBarInternalSubControlRect(option, SC_ScrollBarAddLine));

        QPoint topLeftCorner;
        QPoint botRightCorner;

        if (horizontal) {
            topLeftCorner  = QPoint(topRect.right() + 1, topRect.top());
            botRightCorner = QPoint(bottomRect.left()  - 1, topRect.bottom());
        } else {
            topLeftCorner  = QPoint(topRect.left(),  topRect.bottom() + 1);
            botRightCorner = QPoint(topRect.right(), bottomRect.top() - 1);
        }

        // define rect
        return visualRect(option, QRect(topLeftCorner, botRightCorner));
    }

    case SC_ScrollBarSlider: {
        // handle RTL here to unreflect things if need be
        QRect groove = visualRect(option, subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));
        groove.adjust(0, 2, 1, 1);

        if (sliderOption->minimum == sliderOption->maximum)
            return groove;

        // Figure out how much room there is
        int space(horizontal ? groove.width() : groove.height());

        // Calculate the portion of this space that the slider should occupy
        int sliderSize = space * qreal(sliderOption->pageStep) / (sliderOption->maximum - sliderOption->minimum + sliderOption->pageStep);
        sliderSize = qMax(sliderSize, static_cast<int>(Metrics::ScrollBar_MinSliderHeight));
        sliderSize = qMin(sliderSize, space);

        space -= sliderSize;
        if (space <= 0)
            return groove;

        int pos = qRound(qreal(sliderOption->sliderPosition - sliderOption->minimum) / (sliderOption->maximum - sliderOption->minimum) * space);
        if (sliderOption->upsideDown)
            pos = space - pos;
        if (horizontal)
            return visualRect(option, QRect(groove.left() + pos, groove.top(), sliderSize, groove.height()));
        else
            return visualRect(option, QRect(groove.left(), groove.top() + pos, groove.width(), sliderSize));
    }

    case SC_ScrollBarSubPage: {
        // handle RTL here to unreflect things if need be
        QRect slider = visualRect(option, subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
        QRect groove = visualRect(option, subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

        if (horizontal)
            return visualRect(option, QRect(groove.left(), groove.top(), slider.left() - groove.left(), groove.height()));
        else
            return visualRect(option, QRect(groove.left(), groove.top(), groove.width(), slider.top() - groove.top()));
    }

    case SC_ScrollBarAddPage: {
        // handle RTL here to unreflect things if need be
        QRect slider = visualRect(option, subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
        QRect groove = visualRect(option, subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

        if (horizontal)
            return visualRect(option, QRect(slider.right() + 1, groove.top(), groove.right() - slider.right(), groove.height()));
        else
            return visualRect(option, QRect(groove.left(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom()));
    }

    default:
        return QProxyStyle::subControlRect(CC_ScrollBar, option, subControl, widget);;
    }
}