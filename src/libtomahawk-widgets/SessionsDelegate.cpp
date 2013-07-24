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


#include <SessionsDelegate.h>

#include "playlist/dynamic/GeneratorInterface.h"
#include "utils/TomahawkStyle.h"
#include "utils/TomahawkUtils.h"
#include "utils/TomahawkUtilsGui.h"

#include <QApplication>
#include <QPainter>
#include <QListView>
#include <QDateTime>
#include "libtomahawk/Artist.h"
#include "utils/TomahawkUtils.h"

#include "widgets/RecentlyPlayedPlaylistsModel.h"
#include "playlist/SessionHistoryModel.h" // might be replaced

namespace Tomahawk
{

namespace Widgets
{

SessionDelegate::SessionDelegate()
{
}

void SessionDelegate::setView(QListView *view)
{
    m_view = view ;
}

void
SessionDelegate::onCoverLoaded()
{
    m_view->update();
    //qDebug() << "Passage dans le SLOT onCoverLoaded";
}

void
SessionDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption( &opt, QModelIndex() );
    qApp->style()->drawControl( QStyle::CE_ItemViewItem, &opt, painter );

    if ( option.state & QStyle::State_Selected && option.state & QStyle::State_Active )
    {
        opt.palette.setColor( QPalette::Text, opt.palette.color( QPalette::HighlightedText ) );
    }

    painter->save();
    painter->setRenderHint( QPainter::Antialiasing );
    painter->setPen( opt.palette.color( QPalette::Text ) );

    QTextOption to;
    to.setAlignment( Qt::AlignCenter );
    QFont font = opt.font;
    font.setPointSize( TomahawkUtils::defaultFontSize() - 1 );

    QFont boldFont = font;
    boldFont.setBold( true );
    boldFont.setPointSize( TomahawkUtils::defaultFontSize() );
    QFontMetrics boldFontMetrics( boldFont );

    QFont figFont = boldFont;
    figFont.setPointSize( TomahawkUtils::defaultFontSize() - 1 );

    QPixmap icon;
    QRect pixmapRect = option.rect.adjusted( 10, 14, -option.rect.width() + option.rect.height() - 18, -14 );
    QString artist = index.data(SessionHistoryModel::SessionRole).toString();

    icon = Artist::get(artist)->cover(pixmapRect.size());
    if (  !icon ) {
        icon = TomahawkUtils::defaultPixmap( TomahawkUtils::Playlist, TomahawkUtils::Original, pixmapRect.size() );
    }
//    tDebug() << "ARTISTEEEEE" << artist;
//    tDebug() << "ART OBJ : " << Artist::get(artist)->name() ;
//    QFile file("iconLOLO.png");
//    file.open(QIODevice::WriteOnly);
//    icon.save(&file, "PNG");
//    file.close();
//    tDebug() << "ICONNNNN" ;

    painter->drawPixmap( pixmapRect, TomahawkUtils::createRoundedImage(icon,pixmapRect.size()) );

    connect(Artist::get(artist).data(),SIGNAL(coverChanged()), this, SLOT(onCoverLoaded()));

    QRect r( option.rect.width() - option.fontMetrics.height() * 2.5 - 10, option.rect.top() + option.rect.height() / 3 - option.fontMetrics.height(), option.fontMetrics.height() * 2.5, option.fontMetrics.height() * 2.5 );
    QPixmap avatar;

    if ( avatar.isNull() )
        avatar = TomahawkUtils::defaultPixmap( TomahawkUtils::DefaultSourceAvatar, TomahawkUtils::RoundedCorners, r.size() );
    painter->drawPixmap( r, avatar );

    painter->setFont( font );
    QString author = index.data( SessionHistoryModel::SourceRole).toString();

    const int w = painter->fontMetrics().width( author ) + 2;
    QRect avatarNameRect( opt.rect.width() - 10 - w, r.bottom(), w, opt.rect.bottom() - r.bottom() );
    painter->drawText( avatarNameRect, author, QTextOption( Qt::AlignCenter ) );

    const int leftEdge = opt.rect.width() - qMin( avatarNameRect.left(), r.left() );
    QString descText;
    descText = TomahawkUtils::ageToString( QDateTime::fromTime_t(index.data(SessionHistoryModel::PlaytimeRole).value< unsigned int>()), true );

    QColor c = painter->pen().color();
    if ( !( option.state & QStyle::State_Selected && option.state & QStyle::State_Active ) )
    {
        painter->setPen( QColor( Qt::gray ).darker() );
    }

    QRect rectText = option.rect.adjusted( option.fontMetrics.height() * 4.5, boldFontMetrics.height() + 6, -leftEdge - 10, -8 );
#ifdef Q_WS_MAC
    rectText.adjust( 0, 1, 0, 0 );
#elif defined Q_WS_WIN
    rectText.adjust( 0, 2, 0, 0 );
#endif

    painter->drawText( rectText, descText );
    painter->setPen( c );
    painter->setFont( font );

    painter->setFont( boldFont );
    painter->drawText( option.rect.adjusted( option.fontMetrics.height() * 4, 6, -100, -option.rect.height() + boldFontMetrics.height() + 6 ), index.data(SessionHistoryModel::SessionRole).toString() );

    painter->restore();
}

QSize
SessionDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( option );
    Q_UNUSED( index );

    // Calculates the size for the bold line + 3 normal lines + margins
    int height = 2 * 6; // margins
    QFont font = option.font;
    QFontMetrics fm1( font );
    font.setPointSize( TomahawkUtils::defaultFontSize() - 1 );
    height += fm1.height() * 3;
    font.setPointSize( TomahawkUtils::defaultFontSize() );
    QFontMetrics fm2( font );
    height += fm2.height();

    return QSize( 0, height );
}
}
}
