/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2013, Franck Arrecot <franck.arrecot@gmail.com>
 *   Copyright 2013, Loïc Favier <favier.loic.31@gmail.com>
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

#include "Session.h"
#include "Source.h"

using namespace Tomahawk ;
// using namespace Tomahawk::Widgets ;

Session::Session()
{
    m_queries = QList< query_ptr >();
    m_tracks = QList < QPair <track_ptr , PlaybackLog> >() ;
}

Session::Session(const Session& copy)
{
    m_tracks = QList< QPair<track_ptr , PlaybackLog> >(copy.getTracks()) ;
}

Session::~Session() {

}

//Session* Session::operator<<( Tomahawk::query_ptr query )
//{
//    m_queries << query;
//    return this;
//}

QString
Session::getSessionOwner()
{
    return m_tracks.first().second.source->friendlyName() ;
}

source_ptr
Session::getSessionSource()
{
    return m_tracks.first().second.source ;
}

QList<query_ptr>
Session::getTrackstoQuery()
{
    QList<query_ptr> queries = QList<query_ptr>() ;
    QPair<track_ptr , PlaybackLog> currentTrack ;

    foreach (currentTrack , m_tracks) { queries << currentTrack.first->toQuery() ; };
    return queries ;
}

void
Session::addQuery( QPair<track_ptr , PlaybackLog>& track )
{
    m_tracks << track ;
}

QString
Session::getPredominantArtist()
{
    QString currentArtist = QString();
    QList< QString >aSessionArtists = QList< QString >();
    int currentArtistOccurs = 0;

    //first, get all artists of the session
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <track_ptr , PlaybackLog> track = m_tracks.at(i) ;
        aSessionArtists << track.first->artist() ;
    }

    //then, compute the predominant artist
    for( int i = 0; i < aSessionArtists.count(); i++ )
    {
        if( aSessionArtists.count( aSessionArtists.at(i) ) > currentArtistOccurs )
        {
            currentArtist = aSessionArtists.at(i);
            currentArtistOccurs = aSessionArtists.count( aSessionArtists.at(i) );
        }
    }
    return currentArtist;
}

QList< QString >
Session::getRecurentArtists( const int max ){
    QList< QString > artists = QList< QString >();
    for( int i = 0; i < m_tracks.count() && i < max; i++ )
    {
        QPair <track_ptr , PlaybackLog> track = m_tracks.at(i) ;
        if( artists.count( track.first->artist() ) == 0 )
        {
            artists << track.first->artist();
        }
        else
        {
            i--;
        }
    }
    return artists;
}

QString
Session::getPredominantAlbum()
{
    QString currentAlbum = QString("empty");
    QList< QString >aSessionAlbum = QList< QString >();
    int currentAlbumOccurs = 0;

    //first, get all albums of the session
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <track_ptr , PlaybackLog> track = m_tracks.at(i) ;
        if(track.first->album().size() > 0)
        {
            aSessionAlbum << track.first->album() ;
        }
    }
    //then, compute the predominant album
    for( int i = 0; i < aSessionAlbum.count(); i++ )
    {
        if( aSessionAlbum.count( aSessionAlbum.at(i) ) > currentAlbumOccurs )
        {
            currentAlbum = aSessionAlbum.at(i);
            currentAlbumOccurs = aSessionAlbum.count( aSessionAlbum.at(i) );
        }
    }
    return currentAlbum;
}

int
Session::getStartTime()
{
    return m_tracks.last().second.timestamp ;
}

int
Session::getEndTime()
{
    return m_tracks.first().second.timestamp + m_tracks.first().second.secsPlayed;
}

int
Session::count()
{
    return m_tracks.size() ;
}

bool
Session::trackExist( unsigned int id )
{
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <track_ptr , PlaybackLog> track = m_tracks.at(i) ;
        if( track.first->trackId() == id )
        {
            return true;
        }
    }
    return false;
}

QList < QPair <track_ptr , PlaybackLog> >
Session::getTracks() const
{
    return m_tracks;
}

bool
Session::operator<( Session s )
{
    return this->getEndTime() < s.getEndTime();
}


//template <class Session> bool
//qGreater<Session>::operator()(const Session& s1, const Session& s2) const
//{
//    return s1->getEndTime() < s2->getEndTime();
//}


SessionGreatThan::SessionGreatThan()
{
}

bool
SessionGreatThan::operator ()( Session* s1, Session* s2 )
{
    return s1->getEndTime() > s2->getEndTime();
}

