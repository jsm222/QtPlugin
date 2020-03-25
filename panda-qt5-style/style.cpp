#include "style.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QWidget>
#include <QWindow>

Style::Style() 
    : QProxyStyle("fusion")
{
    // QStyleFactory::create("panda")
}

void Style::polish(QWidget *widget)
{
    if (!widget)
        return;

    // transparent tooltips
    if (widget->inherits("QTipLabel")) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }

    QProxyStyle::polish(widget);
}

void Style::unpolish(QWidget *widget)
{
    QProxyStyle::unpolish(widget);
}

void Style::polish(QApplication *app)
{
    QFont font("Ubuntu");
    font.setPixelSize(15);
    app->setFont(font);
}