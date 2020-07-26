/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 * Copyright (C) 2020 by Vlad Zahorodnii <vlad.zahorodnii@kde.org>       *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#ifndef SHADOWHELPER_H
#define SHADOWHELPER_H

#include "tileset.h"
#include <KWindowShadow>

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QMargins>
#include <QSet>

struct ShadowParams
{
    ShadowParams() = default;

    ShadowParams(const QPoint &offset, int radius, qreal opacity):
        offset(offset),
        radius(radius),
        opacity(opacity)
    {}

    QPoint offset;
    int radius = 0;
    qreal opacity = 0;
};

struct CompositeShadowParams
{
    CompositeShadowParams() = default;

    CompositeShadowParams(
            const QPoint &offset,
            const ShadowParams &shadow1,
            const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2) {}

    bool isNone() const
    { return qMax(shadow1.radius, shadow2.radius) == 0; }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

//* handle shadow pixmaps passed to window manager via X property
class ShadowHelper: public QObject
{
    Q_OBJECT

public:
    //* constructor
    ShadowHelper(QObject*);

    //* destructor
    ~ShadowHelper() override;

    //* shadow params from size enum
    static CompositeShadowParams lookupShadowParams( int shadowSizeEnum );

    //* reset
    void reset();

    //* register widget
    bool registerWidget( QWidget*, bool force = false );

    //* unregister widget
    void unregisterWidget( QWidget* );

    //* event filter
    bool eventFilter( QObject*, QEvent* ) override;

    void setFrameRadius(qreal radius) { _frameRadius = radius; }

    //* shadow tiles
    /** is public because it is also needed for mdi windows */
    // TileSet shadowTiles();

    TileSet shadowTiles(const qreal frameRadius);

protected Q_SLOTS:
    //* unregister widget
    void objectDeleted( QObject* );

protected:
    //* true if widget is a menu
    bool isMenu( QWidget* ) const;

    //* true if widget is a tooltip
    bool isToolTip( QWidget* ) const;

    //* dock widget
    bool isDockWidget( QWidget* ) const;

    //* toolbar
    bool isToolBar( QWidget* ) const;

    //* accept widget
    bool acceptWidget( QWidget* ) const;

    // create shadow tile from pixmap
    KWindowShadowTile::Ptr createTile( const QPixmap& );

    //* installs shadow on given widget in a platform independent way
    // void installShadows( QWidget * );

    void installShadows(QWidget *widget, TileSet shadowTiles);

    //* uninstalls shadow on given widget in a platform independent way
    void uninstallShadows( QWidget * );

    //* gets the shadow margins for the given widget
    QMargins shadowMargins(QWidget*, TileSet) const;

private:
    //* registered widgets
    QSet<QWidget*> _widgets;

    //* managed shadows
    QMap<QWidget*, KWindowShadow*> _shadows;

    qreal _frameRadius = 5;

    //* number of tiles
    enum { numTiles = 8 };

    //* shared shadow tiles
    QVector<KWindowShadowTile::Ptr> _tiles;
};

#endif
