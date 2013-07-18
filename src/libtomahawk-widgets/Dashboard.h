/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Copyright 2010-2011, Jeff Mitchell <jeff@tomahawk-player.org>
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

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QListWidgetItem>
#include <QStyledItemDelegate>

#include "PlaylistInterface.h"

#include "Query.h"
#include "Source.h"
#include "ViewPage.h"

#include "utils/TomahawkUtilsGui.h"

#include "WidgetsDllMacro.h"

class AlbumModel;
class SessionHistoryModel;
class RecentlyPlayedModel;
class OverlayWidget;
class BasicHeader;

namespace Ui
{
    class Dashboard;
}

namespace Tomahawk
{
namespace Widgets
{

class TOMAHAWK_WIDGETS_EXPORT Dashboard : public QWidget, public Tomahawk::ViewPage
{
Q_OBJECT

public:
    Dashboard( QWidget* parent = 0 );
    virtual ~Dashboard();

    virtual QWidget* widget() { return this; }
    virtual Tomahawk::playlistinterface_ptr playlistInterface() const;

    virtual QString title() const { return tr( "Dashboard" ); }
    virtual QString description() const { return tr( "An overview of your recent activity" ); }
    virtual QPixmap pixmap() const;

    virtual bool showInfoBar() const { return true; }
    virtual bool isBeingPlayed() const;

    virtual bool jumpToCurrentTrack();

protected:
    void changeEvent( QEvent* e );

signals:
    void destroyed( QWidget* widget );

public slots:
    void updatePlaylists();
    void updateRecentAdditions();

private slots:
    void onSourcesReady();
    void onSourceAdded( const Tomahawk::source_ptr& source );
    void onPlaylistActivated( const QModelIndex& );
    void onSessionDoubleClicked(const QModelIndex &index);

private:
    Ui::Dashboard *ui;

    RecentlyPlayedModel* m_tracksModel;
    SessionHistoryModel* m_sessionsModel;
    AlbumModel* m_recentAlbumsModel;
    Tomahawk::playlistinterface_ptr m_playlistInterface;
};

}; // Widgets
}; // Tomahawk
#endif // DASHBOARD_H
