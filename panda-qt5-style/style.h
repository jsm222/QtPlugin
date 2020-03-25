#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>
#include "global.h"

class PROXYSTYLESHARED_EXPORT Style : public QProxyStyle
{
    Q_OBJECT

public:
    explicit Style();

    void polish(QWidget *w);
    void unpolish(QWidget *w);
    void polish(QApplication *app);

    void polish(QPalette &palette);

    void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
};

#endif