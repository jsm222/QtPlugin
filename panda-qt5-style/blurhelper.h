#ifndef BLURHELPER_H
#define BLURHELPER_H

#include <QObject>
#include <QTimer>

class BlurHelper : public QObject
{
    Q_OBJECT

public:
    explicit BlurHelper(QObject *parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *e);
    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);
    bool shouldSkip(QWidget *w);

public slots:
    void delayUpdate(QWidget *w, bool updateBlurRegionOnly = false);

private:
    QList<QWidget *> m_blurList;
    QList<QWidget *> m_updateList;
    QTimer *m_timer;
    bool m_blurEnable;
};

#endif // BLURHELPER_H