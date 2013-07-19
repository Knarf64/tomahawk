/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2012, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Copyright 2010-2011, Franck Arrecot <franck.arrecot@gmail.com>
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


#include <QMetaType>
#include <QMimeData>
#include <QTreeView>
#include <qstring.h>

#include "Source.h"
#include "SourceList.h"
#include "SessionHistoryModel.h"
#include "database/Database.h"
#include "database/DatabaseCommand_PlaybackHistorySessions.h"
#include "database/DatabaseCommand_GenericSelect.h"
#include "PlayableItem.h"
#include "utils/TomahawkUtils.h"
#include "utils/Logger.h"

#define HISTORY_SESSION_ITEMS 100

const static int MAX_TIME_BETWEEN_TRACKS = 5 * 60;
const static int MIN_SESSION_COUNT = 1;

using namespace Tomahawk;

SessionHistoryModel::SessionHistoryModel( QObject* parent )
    //: PlaylistModel( parent )
      : QAbstractListModel( parent )
    , m_limit( HISTORY_SESSION_ITEMS )
{
    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), this, SLOT( onSourceAdded( Tomahawk::source_ptr ) ), Qt::QueuedConnection );
}


SessionHistoryModel::~SessionHistoryModel()
{
}

void
SessionHistoryModel::loadHistory()
{
    if ( rowCount( QModelIndex() ) )
    {
    }
    retrievePlayBackSongs() ;
}

void
SessionHistoryModel::retrievePlayBackSongs()
{
    DatabaseCommand_PlaybackHistorySessions* cmd = new DatabaseCommand_PlaybackHistorySessions( m_source );
    cmd->setLimit( m_limit );

    // Collect the return from db into SessionsFromQueries to build sessions
    connect( cmd, SIGNAL( tracksSession( QList<Tomahawk::track_ptr> , QList<Tomahawk::PlaybackLog>  ) ),
                    SLOT( sessionsFromQueries( QList<Tomahawk::track_ptr> ,
                                               QList<Tomahawk::PlaybackLog> ) ), Qt::QueuedConnection );

    Database::instance()->enqueue( QSharedPointer<DatabaseCommand>( cmd ) );
}

void
SessionHistoryModel::onSourcesReady()
{
    Q_ASSERT( m_source.isNull() );

    foreach ( const source_ptr& source, SourceList::instance()->sources() )
        onSourceAdded( source );

    loadHistory();
}


void
SessionHistoryModel::setSource( const Tomahawk::source_ptr& source )
{
    m_source = source;
    if ( source.isNull() )
    {
        if ( SourceList::instance()->isReady() )
            onSourcesReady();
        else
            connect( SourceList::instance(), SIGNAL( ready() ), SLOT( onSourcesReady() ) );

        connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );
    }
    else
    {
        onSourceAdded( source );
        loadHistory();
    }
}


void
SessionHistoryModel::onSourceAdded( const Tomahawk::source_ptr& source )
{
    connect( source.data(), SIGNAL( playbackFinished( Tomahawk::track_ptr , Tomahawk::PlaybackLog ) ), SLOT( onPlaybackFinished( const Tomahawk::track_ptr, const Tomahawk::PlaybackLog) ), Qt::UniqueConnection );
}

void
SessionHistoryModel::onPlaybackFinished( const Tomahawk::track_ptr& track, const Tomahawk::PlaybackLog& log )
{
    loadHistory();
    //emit dataChanged(); TODO
}


void
SessionHistoryModel::sessionsFromQueries( const QList<Tomahawk::track_ptr>& tracks,
                                          const QList<Tomahawk::PlaybackLog>& logs )
{
    tDebug() << "Session Calculate From Queries Beginin" ;

    //if ( !queries.count() )
    if ( !tracks.count() )
    {
        //emit itemCountChanged( rowCount( QModelIndex() ) ); // TODO : Replace
        //finishLoading(); // TODO : Replace
        return;
    }
    else tDebug() << "Session Calculate starting" ;

    tDebug() << "Number of Queries retireved" <<tracks.count() ;


    unsigned int lastTimeStamp = 0;
    QString currentPeer = QString();
    Session* oneSession = new Session();
    QList< Session* > sessions = QList< Session* >();
    QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> currentTrack ;

    for( int i = 0 ; i < tracks.count() ; i++ )
    {
        currentTrack = QPair<Tomahawk::track_ptr , Tomahawk::PlaybackLog>( tracks.at(i) , logs.at(i) ) ;

        if( lastTimeStamp == 0 )
        {
            lastTimeStamp = currentTrack.second.timestamp  ;
        }

        if( currentPeer == "" )
        {
            currentPeer = currentTrack.second.source->friendlyName() ;
        }

        if( lastTimeStamp - currentTrack.second.timestamp < MAX_TIME_BETWEEN_TRACKS && currentPeer == currentTrack.second.source->friendlyName()  )
        {
            //it's the same session, we add it
            if( !oneSession->trackExist( currentTrack.first->trackId() ) )
            {
                oneSession->addQuery(currentTrack);
            }
        }
        else
        {
            //add the current session in the session list only if session contains MIN_SESSION_COUNT at least
            if( oneSession->count() >= MIN_SESSION_COUNT )
            {
                sessions << oneSession;

            }

            //new session
            oneSession = new Session();
            currentPeer = currentTrack.second.source->friendlyName() ;
            //add the current query in the new session
            oneSession->addQuery(currentTrack);
        }
        lastTimeStamp = currentTrack.second.timestamp + currentTrack.second.secsPlayed ;
    }

    //add the last session in the session list only if session contains MIN_SESSION_COUNT at least
    if( oneSession->count() >= MIN_SESSION_COUNT )
    {
        sessions << oneSession;

    }

    //sort sessions list
    qSort(sessions.begin(), sessions.end(), SessionGreatThan());

    // insert the sessions inside the model
    feedModelWithSessions( sessions );

    //debug : show sessions
    tDebug() << "We have calculate " << sessions.count() << " sessions : ";
    for( int i = 0 ; i < sessions.count() ; i++ )
    {
        tDebug() << "session " << ( i + 1 ) << " : " << sessions.at(i)->getSessionOwner()
                 << " played " << sessions.at(i)->getPredominantAlbum() << " of "
                 << sessions.at(i)->getPredominantArtist() << " from " << sessions.at(i)->getStartTime()
                 << " to " << sessions.at(i)->getEndTime() << " [" <<  sessions.at(i)->count() << "]";

        QPair<Tomahawk::track_ptr,Tomahawk::PlaybackLog> track ;
        foreach ( track , sessions.at(i)->getTracks() )
        {
            tDebug() << "   - " << track.first->artist() << track.first->track() << " from " << track.second.source->friendlyName()  << " played at " << track.second.timestamp ;
        };
    }
}

void
SessionHistoryModel::feedModelWithSessions ( const QList< Session* >& sessions)
{
    if (sessions.count()) // TODO : add tracks as child of the session name
    {
        if (sessions.count())
        {
            beginResetModel();
            m_sessionslist.clear();
            m_sessionslist = sessions ;
            endResetModel();
        }
    }
}

QVariant
SessionHistoryModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || !hasIndex( index.row(), index.column(), index.parent() ) )
        return QVariant();

    Session* mySession =  m_sessionslist.at(index.row()) ;

    switch( role )
    {
    case SourceRole:
    {
        return QVariant(mySession->getSessionOwner()) ;
    }
    case SessionRole:
    {
        return QVariant(mySession->getPredominantArtist()) ;
    }
    case PlaytimeRole:
    {
        return QVariant(mySession->getEndTime()) ;
    }
    case SessionItemRole:
    {
        return QVariant::fromValue(mySession);
    }
    case SizeRole:
    {
        return QVariant(mySession->count()) ;
    }
    default:
        return QVariant();
    }
}

int
SessionHistoryModel::rowCount( const QModelIndex& ) const
{
    return m_sessionslist.count() ;
}

