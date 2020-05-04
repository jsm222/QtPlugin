#include "style.h"
#include <QTextLayout>
#include <private/qtextengine_p.h>

static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth, int maxHeight = -1, int *lastVisibleLine = nullptr)
{
    if (lastVisibleLine)
        *lastVisibleLine = -1;
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    int i = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
        // we assume that the height of the next line is the same as the current one
        if (maxHeight > 0 && lastVisibleLine && height + line.height() > maxHeight) {
            const QTextLine nextLine = textLayout.createLine();
            *lastVisibleLine = nextLine.isValid() ? i : -1;
            break;
        }
        ++i;
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}

QString Style::calculateElidedText(const QString &text, const QTextOption &textOption,
                            const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                            Qt::TextElideMode textElideMode, int flags,
                            bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition) const
{
    QTextLayout textLayout(text, font);
    textLayout.setTextOption(textOption);

    // In AlignVCenter mode when more than one line is displayed and the height only allows
    // some of the lines it makes no sense to display those. From a users perspective it makes
    // more sense to see the start of the text instead something inbetween.
    const bool vAlignmentOptimization = paintStartPosition && valign.testFlag(Qt::AlignVCenter);

    int lastVisibleLine = -1;
    viewItemTextLayout(textLayout, textRect.width(), vAlignmentOptimization ? textRect.height() : -1, &lastVisibleLine);

    const QRectF boundingRect = textLayout.boundingRect();
    // don't care about LTR/RTL here, only need the height
    const QRect layoutRect = QStyle::alignedRect(Qt::LayoutDirectionAuto, valign,
                                                 boundingRect.size().toSize(), textRect);

    if (paintStartPosition)
        *paintStartPosition = QPointF(textRect.x(), layoutRect.top());

    QString ret;
    qreal height = 0;
    const int lineCount = textLayout.lineCount();
    for (int i = 0; i < lineCount; ++i) {
        const QTextLine line = textLayout.lineAt(i);
        height += line.height();

        // above visible rect
        if (height + layoutRect.top() <= textRect.top()) {
            if (paintStartPosition)
                paintStartPosition->ry() += line.height();
            continue;
        }

        const int start = line.textStart();
        const int length = line.textLength();
        const bool drawElided = line.naturalTextWidth() > textRect.width();
        bool elideLastVisibleLine = lastVisibleLine == i;
        if (!drawElided && i + 1 < lineCount && lastVisibleLineShouldBeElided) {
            const QTextLine nextLine = textLayout.lineAt(i + 1);
            const int nextHeight = height + nextLine.height() / 2;
            // elide when less than the next half line is visible
            if (nextHeight + layoutRect.top() > textRect.height() + textRect.top())
                elideLastVisibleLine = true;
        }

        QString text = textLayout.text().mid(start, length);
        if (drawElided || elideLastVisibleLine) {
            if (elideLastVisibleLine) {
                if (text.endsWith(QChar::LineSeparator))
                    text.chop(1);
                text += QChar(0x2026);
            }
            const QStackTextEngine engine(text, font);
            ret += engine.elidedText(textElideMode, textRect.width(), flags);

            // no newline for the last line (last visible or real)
            // sometimes drawElided is true but no eliding is done so the text ends
            // with QChar::LineSeparator - don't add another one. This happened with
            // arabic text in the testcase for QTBUG-72805
            if (i < lineCount - 1 &&
                    !ret.endsWith(QChar::LineSeparator))
                ret += QChar::LineSeparator;
        } else {
            ret += text;
        }

        // below visible text, can stop
        if ((height + layoutRect.top() >= textRect.bottom()) ||
                (lastVisibleLine >= 0 && lastVisibleLine == i))
            break;
    }
    return ret;
}

void Style::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const
{
    const QWidget *widget = option->widget;
    const int textMargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);
    textOption.setAlignment(QStyle::visualAlignment(option->direction, option->displayAlignment));

    QPointF paintPosition;
    const QString newText = calculateElidedText(option->text, textOption,
                                                option->font, textRect, option->displayAlignment,
                                                option->textElideMode, 0,
                                                true, &paintPosition);

    QTextLayout textLayout(newText, option->font);
    textLayout.setTextOption(textOption);
    viewItemTextLayout(textLayout, textRect.width());
    textLayout.draw(p, paintPosition);
}