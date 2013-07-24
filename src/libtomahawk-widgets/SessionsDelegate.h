/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2013, Franck Arrecot <franck.arrecot@gmail.com>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOMAHAWK_WIDGETS_SESSIONSDELEGATE_H
#define TOMAHAWK_WIDGETS_SESSIONSDELEGATE_H

#include <QStyledItemDelegate>
#include "WidgetsDllMacro.h"

class QListView;

namespace Tomahawk
{

namespace Widgets
{

class TOMAHAWK_WIDGETS_EXPORT SessionDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    SessionDelegate();
    void setView(QListView *view);
private slots:
        void onCoverLoaded();
protected:
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    QListView *m_view ;
};

} // Widgets

} // Tomahawk

#endif // TOMAHAWK_WIDGETS_SESSIONSDELEGATE_H
