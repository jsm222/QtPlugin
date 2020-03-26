#include "style.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QWidget>
#include <QWindow>
#include <QAbstractItemView>

#include <QPainter>

Style::Style() 
    : QProxyStyle("fusion")
{
    // QStyleFactory::create("panda")
}

void Style::polish(QWidget *w)
{
    if (!w)
        return;

    // transparent tooltips
    if (w->inherits("QTipLabel")) {
        w->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(w)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    QProxyStyle::polish(w);
}

void Style::unpolish(QWidget *w)
{
    QProxyStyle::unpolish(w);
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
    palette.setBrush(QPalette::Window, windowBg);
}

void Style::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {

    case PE_Frame: {
        break;
    }

    case PE_PanelLineEdit: {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(242, 242, 242));
            painter->drawRoundedRect(panel->rect, 6, 6);
        }
        painter->restore();
        break;
    }

    case PE_FrameLineEdit: {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QRectF frameRect(option->rect);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(242, 242, 242));
        painter->drawRoundedRect(frameRect.adjusted(0.5, 0.5, -0.5, -0.5), 6, 6);
        painter->restore();
        break;
    }

    case PE_PanelButtonCommand: {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(242, 242, 242));

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

void Style::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}