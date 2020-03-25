#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>
#include "global.h"

class PROXYSTYLESHARED_EXPORT Style : public QProxyStyle
{
    Q_OBJECT

public:
    explicit Style();

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

    void polish(QApplication *app);
};

#endif