/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Copyright 2011, Leo Franchi <lfranchi@kde.org>
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

#include "Dashboard.h"
#include "ui_Dashboard.h"

#include "ViewManager.h"
#include "SourceList.h"
#include "TomahawkSettings.h"
#include "widgets/RecentPlaylistsModel.h"
#include "widgets/RecentlyPlayedPlaylistsModel.h"
#include "MetaPlaylistInterface.h"
#include "PlaylistDelegate.h"
#include "audio/AudioEngine.h"
#include "playlist/AlbumModel.h"
#include "playlist/RecentlyPlayedModel.h"
#include "playlist/dynamic/GeneratorInterface.h"
#include "widgets/OverlayWidget.h"
#include "widgets/BasicHeader.h"
#include "utils/ImageRegistry.h"
#include "utils/AnimatedSpinner.h"
#include "utils/TomahawkStyle.h"
#include "utils/TomahawkUtilsGui.h"
#include "utils/Logger.h"

#include <QPainter>
#include <QScrollArea>

#define HISTORY_PLAYLIST_ITEMS 10
#define HISTORY_TRACK_ITEMS 15

using namespace Tomahawk;
using namespace Tomahawk::Widgets;


Dashboard::Dashboard( QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::Dashboard )
{
    QWidget* widget = new QWidget;
    ui->setupUi( widget );

    ui->lineAbove->setStyleSheet( QString( "QFrame { border: 1px solid black; }" ) );
    ui->lineBelow->setStyleSheet( QString( "QFrame { border: 1px solid %1; }" ).arg( TomahawkStyle::HEADER_BACKGROUND.name() ) );

    {
        // TO REMOVE when replaced
        m_tracksModel = new RecentlyPlayedModel( ui->tracksView, HISTORY_TRACK_ITEMS );
        ui->tracksView->proxyModel()->setStyle( PlayableProxyModel::ShortWithAvatars );
        ui->tracksView->overlay()->setEnabled( false );
        ui->tracksView->setPlaylistModel( m_tracksModel );
        ui->tracksView->setAutoResize( true );
        ui->tracksView->setAlternatingRowColors( false );
        m_tracksModel->setSource( source_ptr() );

        /*
        m_sessionsModel = new SessionHistoryModel(ui->sessionsView) ; // change to add the argument !
        ui->sessionsView->setItemDelegate( new SessionDelegate(ui->sessionsView) );
        ui->sessionsView->setModel( m_sessionsModel );
        m_sessionsModel->setSource( source_ptr() );
        */

        QPalette p = ui->tracksView->palette();
        p.setColor( QPalette::Text, TomahawkStyle::PAGE_TRACKLIST_TRACK_SOLVED );
        p.setColor( QPalette::BrightText, TomahawkStyle::PAGE_TRACKLIST_TRACK_UNRESOLVED );
        p.setColor( QPalette::Foreground, TomahawkStyle::PAGE_TRACKLIST_NUMBER );
        p.setColor( QPalette::Highlight, TomahawkStyle::PAGE_TRACKLIST_HIGHLIGHT );
        p.setColor( QPalette::HighlightedText, TomahawkStyle::PAGE_TRACKLIST_HIGHLIGHT_TEXT );

        ui->tracksView->setPalette( p );
        ui->tracksView->setFrameShape( QFrame::NoFrame );
        ui->tracksView->setAttribute( Qt::WA_MacShowFocusRect, 0 );
        ui->tracksView->setStyleSheet( "QTreeView { background-color: transparent; }" );
        TomahawkStyle::stylePageFrame( ui->trackFrame );
    }

    {
        RecentPlaylistsModel* model = new RecentPlaylistsModel( HISTORY_PLAYLIST_ITEMS, this );

        ui->playlistWidget->setFrameShape( QFrame::NoFrame );
        ui->playlistWidget->setAttribute( Qt::WA_MacShowFocusRect, 0 );
        ui->playlistWidget->setItemDelegate( new PlaylistDelegate() );
        ui->playlistWidget->setModel( model );
        ui->playlistWidget->overlay()->resize( 380, 86 );
        ui->playlistWidget->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

        QPalette p = ui->playlistWidget->palette();
        p.setColor( QPalette::Text, TomahawkStyle::HEADER_TEXT );
        p.setColor( QPalette::BrightText, TomahawkStyle::HEADER_TEXT );
        p.setColor( QPalette::Foreground, TomahawkStyle::HEADER_TEXT );
        p.setColor( QPalette::Highlight, TomahawkStyle::HEADER_TEXT );
        p.setColor( QPalette::HighlightedText, TomahawkStyle::HEADER_BACKGROUND );

        ui->playlistWidget->setPalette( p );
        ui->playlistWidget->setMinimumHeight( 400 );
        ui->playlistWidget->setStyleSheet( "QListView { background-color: transparent; }" );
        TomahawkStyle::styleScrollBar( ui->playlistWidget->verticalScrollBar() );
        TomahawkStyle::stylePageFrame( ui->playlistFrame );

        updatePlaylists();
        connect( ui->playlistWidget, SIGNAL( activated( QModelIndex ) ), SLOT( onPlaylistActivated( QModelIndex ) ) );
        connect( model, SIGNAL( emptinessChanged( bool ) ), this, SLOT( updatePlaylists() ) );
    }

    {
        m_recentAlbumsModel = new AlbumModel( ui->additionsView );
        ui->additionsView->setPlayableModel( m_recentAlbumsModel );
        ui->additionsView->proxyModel()->sort( -1 );

        ui->additionsView->setStyleSheet( "QListView { background-color: transparent; }" );
        TomahawkStyle::stylePageFrame( ui->additionsFrame );
        TomahawkStyle::styleScrollBar( ui->additionsView->verticalScrollBar() );
    }

    {
        QFont f = ui->label->font();
        f.setFamily( "Pathway Gothic One" );

        QPalette p = ui->label->palette();
        p.setColor( QPalette::Foreground, TomahawkStyle::PAGE_CAPTION );

        ui->label->setFont( f );
        ui->label_2->setFont( f );
        ui->label->setPalette( p );
        ui->label_2->setPalette( p );
    }

    {
        QFont f = ui->playlistLabel->font();
        f.setFamily( "Pathway Gothic One" );

        QPalette p = ui->playlistLabel->palette();
        p.setColor( QPalette::Foreground, TomahawkStyle::HEADER_TEXT );

        ui->playlistLabel->setFont( f );
        ui->playlistLabel->setPalette( p );
    }

    {
        QScrollArea* area = new QScrollArea();
        area->setWidgetResizable( true );
        area->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        area->setWidget( widget );

        QPalette pal = palette();
        pal.setBrush( backgroundRole(), TomahawkStyle::HEADER_BACKGROUND );
        area->setPalette( pal );
        area->setAutoFillBackground( true );
        area->setFrameShape( QFrame::NoFrame );
        area->setAttribute( Qt::WA_MacShowFocusRect, 0 );

        QVBoxLayout* layout = new QVBoxLayout();
        layout->addWidget( area );
        setLayout( layout );
        TomahawkUtils::unmarginLayout( layout );
    }

    {
        QPalette pal = palette();
        pal.setBrush( backgroundRole(), TomahawkStyle::PAGE_BACKGROUND );
        ui->widget->setPalette( pal );
        ui->widget->setAutoFillBackground( true );
    }

    MetaPlaylistInterface* mpl = new MetaPlaylistInterface();
    mpl->addChildInterface( ui->tracksView->playlistInterface() );
    mpl->addChildInterface( ui->additionsView->playlistInterface() );
    m_playlistInterface = playlistinterface_ptr( mpl );

    connect( SourceList::instance(), SIGNAL( ready() ), SLOT( onSourcesReady() ) );
    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );
}


Dashboard::~Dashboard()
{
    delete ui;
}


Tomahawk::playlistinterface_ptr
Dashboard::playlistInterface() const
{
    return m_playlistInterface;
}


bool
Dashboard::jumpToCurrentTrack()
{
    if ( ui->tracksView->jumpToCurrentTrack() )
        return true;

    if ( ui->additionsView->jumpToCurrentTrack() )
        return true;

    return false;
}


bool
Dashboard::isBeingPlayed() const
{
    if ( ui->additionsView->isBeingPlayed() )
        return true;

    return AudioEngine::instance()->currentTrackPlaylist() == ui->tracksView->playlistInterface();
}


void
Dashboard::onSourcesReady()
{
    foreach ( const source_ptr& source, SourceList::instance()->sources() )
        onSourceAdded( source );

    updateRecentAdditions();
}


void
Dashboard::onSourceAdded( const Tomahawk::source_ptr& source )
{
    connect( source->dbCollection().data(), SIGNAL( changed() ), SLOT( updateRecentAdditions() ), Qt::UniqueConnection );
}


void
Dashboard::updateRecentAdditions()
{
    m_recentAlbumsModel->addFilteredCollection( collection_ptr(), 20, DatabaseCommand_AllAlbums::ModificationTime, true );
}


void
Dashboard::updatePlaylists()
{
    int num = ui->playlistWidget->model()->rowCount( QModelIndex() );
    if ( num == 0 )
    {
        ui->playlistWidget->overlay()->setText( tr( "No recently created playlists in your network." ) );
        ui->playlistWidget->overlay()->show();
    }
    else
        ui->playlistWidget->overlay()->hide();
}


void
Dashboard::onPlaylistActivated( const QModelIndex& item )
{
    Tomahawk::playlist_ptr pl = item.data( RecentlyPlayedPlaylistsModel::PlaylistRole ).value< Tomahawk::playlist_ptr >();
    if ( Tomahawk::dynplaylist_ptr dynplaylist = pl.dynamicCast< Tomahawk::DynamicPlaylist >() )
        ViewManager::instance()->show( dynplaylist );
    else
        ViewManager::instance()->show( pl );
}

void Dashboard::onSessionDoubleClicked(const QModelIndex &index)
{
    /*
    // Retrieve Session and fill the playlist
    Session* mySession = index.data(SessionHistoryModel::SessionItemRole).value<Session*>() ;
    source_ptr author = mySession->getSessionSource() ;
    QString title = "Session from : "+author->friendlyName() ;
    QList<Tomahawk::query_ptr> queries =  mySession->getTrackstoQuery() ;

    Tomahawk::ViewPage* view = ViewManager::instance()->createPageForList( title, queries );
    ViewManager::instance()->show( view );
*/
}

void
Dashboard::changeEvent( QEvent* e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            ui->retranslateUi( this );
            break;

        default:
            break;
    }
}


QPixmap
Dashboard::pixmap() const
{
    return ImageRegistry::instance()->pixmap( RESPATH "images/dashboard.svg", QSize( 0, 0 ) );
}
