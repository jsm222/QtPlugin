/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "styleanimation.h"

#ifndef QT_NO_ANIMATION

#include <qcoreapplication.h>
#include <qwidget.h>
#include <qevent.h>

static const qreal ScrollBarFadeOutDuration = 200.0;
static const qreal ScrollBarFadeOutDelay = 450.0;

StyleAnimation::StyleAnimation(QObject *target) : QAbstractAnimation(target),
    _delay(0), _duration(-1), _startTime(QTime::currentTime()), _fps(ThirtyFps), _skip(0)
{
}

StyleAnimation::~StyleAnimation()
{
}

QObject *StyleAnimation::target() const
{
    return parent();
}

int StyleAnimation::duration() const
{
    return _duration;
}

void StyleAnimation::setDuration(int duration)
{
    _duration = duration;
}

int StyleAnimation::delay() const
{
    return _delay;
}

void StyleAnimation::setDelay(int delay)
{
    _delay = delay;
}

QTime StyleAnimation::startTime() const
{
    return _startTime;
}

void StyleAnimation::setStartTime(const QTime &time)
{
    _startTime = time;
}

StyleAnimation::FrameRate StyleAnimation::frameRate() const
{
    return _fps;
}

void StyleAnimation::setFrameRate(FrameRate fps)
{
    _fps = fps;
}

void StyleAnimation::updateTarget()
{
    QEvent event(QEvent::StyleAnimationUpdate);
    event.setAccepted(false);
    QCoreApplication::sendEvent(target(), &event);
    if (!event.isAccepted())
        stop();
}

void StyleAnimation::start()
{
    _skip = 0;
    QAbstractAnimation::start(DeleteWhenStopped);
}

bool StyleAnimation::isUpdateNeeded() const
{
    return currentTime() > _delay;
}

void StyleAnimation::updateCurrentTime(int)
{
    if (++_skip >= _fps) {
        _skip = 0;
        if (target() && isUpdateNeeded())
            updateTarget();
    }
}

ProgressStyleAnimation::ProgressStyleAnimation(int speed, QObject *target) :
    StyleAnimation(target), _speed(speed), _step(-1)
{
}

int ProgressStyleAnimation::animationStep() const
{
    return currentTime() / (1000.0 / _speed);
}

int ProgressStyleAnimation::progressStep(int width) const
{
    int step = animationStep();
    int progress = (step * width / _speed) % width;
    if (((step * width / _speed) % (2 * width)) >= width)
        progress = width - progress;
    return progress;
}

int ProgressStyleAnimation::speed() const
{
    return _speed;
}

void ProgressStyleAnimation::setSpeed(int speed)
{
    _speed = speed;
}

bool ProgressStyleAnimation::isUpdateNeeded() const
{
    if (StyleAnimation::isUpdateNeeded()) {
        int current = animationStep();
        if (_step == -1 || _step != current)
        {
            _step = current;
            return true;
        }
    }
    return false;
}

NumberStyleAnimation::NumberStyleAnimation(QObject *target) :
    StyleAnimation(target), _start(0.0), _end(1.0), _prev(0.0)
{
    setDuration(250);
}

qreal NumberStyleAnimation::startValue() const
{
    return _start;
}

void NumberStyleAnimation::setStartValue(qreal value)
{
    _start = value;
}

qreal NumberStyleAnimation::endValue() const
{
    return _end;
}

void NumberStyleAnimation::setEndValue(qreal value)
{
    _end = value;
}

qreal NumberStyleAnimation::currentValue() const
{
    qreal step = qreal(currentTime() - delay()) / (duration() - delay());
    return _start + qMax(qreal(0), step) * (_end - _start);
}

bool NumberStyleAnimation::isUpdateNeeded() const
{
    if (StyleAnimation::isUpdateNeeded()) {
        qreal current = currentValue();
        if (!qFuzzyCompare(_prev, current))
        {
            _prev = current;
            return true;
        }
    }
    return false;
}

BlendStyleAnimation::BlendStyleAnimation(Type type, QObject *target) :
    StyleAnimation(target), _type(type)
{
    setDuration(250);
}

QImage BlendStyleAnimation::startImage() const
{
    return _start;
}

void BlendStyleAnimation::setStartImage(const QImage& image)
{
    _start = image;
}

QImage BlendStyleAnimation::endImage() const
{
    return _end;
}

void BlendStyleAnimation::setEndImage(const QImage& image)
{
    _end = image;
}

QImage BlendStyleAnimation::currentImage() const
{
    return _current;
}

/*! \internal
    A helper function to blend two images.
    The result consists of ((alpha)*startImage) + ((1-alpha)*endImage)
*/
static QImage blendedImage(const QImage &start, const QImage &end, float alpha)
{
    if (start.isNull() || end.isNull())
        return QImage();

    QImage blended;
    const int a = qRound(alpha*256);
    const int ia = 256 - a;
    const int sw = start.width();
    const int sh = start.height();
    const int bpl = start.bytesPerLine();
    switch (start.depth()) {
    case 32:
        {
            blended = QImage(sw, sh, start.format());
            blended.setDevicePixelRatio(start.devicePixelRatio());
            uchar *mixed_data = blended.bits();
            const uchar *back_data = start.bits();
            const uchar *front_data = end.bits();
            for (int sy = 0; sy < sh; sy++) {
                quint32* mixed = (quint32*)mixed_data;
                const quint32* back = (const quint32*)back_data;
                const quint32* front = (const quint32*)front_data;
                for (int sx = 0; sx < sw; sx++) {
                    quint32 bp = back[sx];
                    quint32 fp = front[sx];
                    mixed[sx] =  qRgba ((qRed(bp)*ia + qRed(fp)*a)>>8,
                                        (qGreen(bp)*ia + qGreen(fp)*a)>>8,
                                        (qBlue(bp)*ia + qBlue(fp)*a)>>8,
                                        (qAlpha(bp)*ia + qAlpha(fp)*a)>>8);
                }
                mixed_data += bpl;
                back_data += bpl;
                front_data += bpl;
            }
        }
    default:
        break;
    }
    return blended;
}

void BlendStyleAnimation::updateCurrentTime(int time)
{
    StyleAnimation::updateCurrentTime(time);

    float alpha = 1.0;
    if (duration() > 0) {
        if (_type == Pulse) {
            time = time % duration() * 2;
            if (time > duration())
                time = duration() * 2 - time;
        }

        alpha = time / static_cast<float>(duration());

        if (_type == Transition && time > duration()) {
            alpha = 1.0;
            stop();
        }
    } else if (time > 0) {
        stop();
    }

    _current = blendedImage(_start, _end, alpha);
}

ScrollbarStyleAnimation::ScrollbarStyleAnimation(Mode mode, QObject *target) : NumberStyleAnimation(target), _mode(mode), _active(false)
{
    switch (mode) {
    case Activating:
        setDuration(ScrollBarFadeOutDuration);
        setStartValue(0.0);
        setEndValue(1.0);
        break;
    case Deactivating:
        setDuration(ScrollBarFadeOutDelay + ScrollBarFadeOutDuration);
        setDelay(ScrollBarFadeOutDelay);
        setStartValue(1.0);
        setEndValue(0.0);
        break;
    }
}

ScrollbarStyleAnimation::Mode ScrollbarStyleAnimation::mode() const
{
    return _mode;
}

bool ScrollbarStyleAnimation::wasActive() const
{
    return _active;
}

void ScrollbarStyleAnimation::setActive(bool active)
{
    _active = active;
}

#endif //QT_NO_ANIMATION