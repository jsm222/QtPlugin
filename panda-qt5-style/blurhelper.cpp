#include "blurhelper.h"

// KF5
#include <KWindowEffects>

// Qt
#include <QWidget>
#include <QVariant>
#include <QEvent>

BlurHelper::BlurHelper(QObject *parent) 
    : QObject(parent),
      m_timer(new QTimer(this)),
      m_blurEnable(true)
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
}

bool BlurHelper::eventFilter(QObject *obj, QEvent *e)
{
    if (!m_blurEnable)
        return false;

    QWidget *widget = qobject_cast<QWidget *>(obj);
    if (widget->winId() <= 0)
        return false;

    switch (e->type()) {
    case QEvent::UpdateRequest: {
        delayUpdate(widget, true);
        break;
    }
    case QEvent::LayoutRequest: {
        delayUpdate(widget);
        break;
    }
    case QEvent::Hide: {
        KWindowEffects::enableBlurBehind(widget->winId(), false);
    }
    default: break;
    }

    return false;
}

void BlurHelper::registerWidget(QWidget *widget)
{
    if (shouldSkip(widget))
        return;

    if (!m_blurList.contains(widget)) {
        m_blurList << widget;
        connect(widget, &QWidget::destroyed, this, [=] { unregisterWidget(widget); });
    }

    widget->removeEventFilter(this);
    widget->installEventFilter(this);

    if (!widget->mask().isEmpty()) {
        widget->update(widget->mask());
    } else {
        widget->update();
    }
}

void BlurHelper::unregisterWidget(QWidget *widget)
{
    if (shouldSkip(widget))
        return;

    m_blurList.removeOne(widget);
    widget->removeEventFilter(this);
    KWindowEffects::enableBlurBehind(widget->winId(), false);
}

bool BlurHelper::shouldSkip(QWidget *w)
{
    bool skip = false;
    if (w->inherits("QComboBoxPrivateContainer"))
        return true;

    return skip;
}

void BlurHelper::delayUpdate(QWidget *w, bool updateBlurRegionOnly)
{
    if (w->winId() <= 0)
        return;

    m_updateList.append(w);
    if (!m_timer->isActive()) {
        for (auto widget : m_updateList) {
            if (!widget)
                continue;

            if (widget->winId() <= 0)
                continue;

            bool hasMask = false;
            if (widget->mask().isNull())
                hasMask = true;

            QVariant regionValue = widget->property("blurRegion");
            QRegion region = qvariant_cast<QRegion>(regionValue);

            if (widget->inherits("QMenu")) {
                QPainterPath path;
                path.addRoundedRect(widget->rect().adjusted(9, 9, -9, -9), 10, 10);
                KWindowEffects::enableBlurBehind(widget->winId(), true, path.toFillPolygon().toPolygon());
                if (!updateBlurRegionOnly)
                    widget->update();
                break;
            }

            if (!hasMask && region.isEmpty())
                break;

            if (!region.isEmpty()) {
                KWindowEffects::enableBlurBehind(widget->winId(), true, region);
                if (!updateBlurRegionOnly)
                    widget->update();
            } else {
                KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
                if (!updateBlurRegionOnly)
                    widget->update(widget->mask());
            }
        }
        m_updateList.clear();
    } else {
        m_timer->start();
    }
}