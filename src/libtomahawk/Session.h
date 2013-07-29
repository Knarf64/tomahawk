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

#ifndef SESSION_H
#define SESSION_H

#include <QtCore/QObject>
#include <QWidgetList>
#include "Query.h"
#include "Track.h"
#include "utils/TomahawkUtils.h"

namespace Tomahawk {
// namespace Widgets {

class Session : public QObject
{
Q_OBJECT

public:
    explicit Session();
    explicit Session(const Session& copy);
    virtual ~Session();

    bool operator<(const Session s);
    void addQuery( QPair<track_ptr , PlaybackLog>& track ) ;

    QString getSessionOwner();
    QString getPredominantArtist();
    QList< QString > getRecurentArtists( const int max = 3);
    QString getPredominantAlbum();

    source_ptr getSessionSource();
    QList<query_ptr> getTrackstoQuery();

    QList < QPair <track_ptr , PlaybackLog> >getTracks() const;

    int getStartTime();
    int getEndTime();
    int count();

    bool trackExist( unsigned int id );

private:
    QList < QPair <track_ptr , PlaybackLog> > m_tracks ;
    QList< query_ptr > m_queries;

};

class SessionGreatThan
{
public:
    explicit SessionGreatThan();
    bool operator()(Session *s1, Session *s2 );
};
}
Q_DECLARE_METATYPE ( Tomahawk::Session* )
Q_DECLARE_METATYPE ( Tomahawk::Session)

#endif // SESSION_H
