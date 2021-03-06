/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "playlist.h"

class PlaylistManagerPrivate;
class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QObject *parent = 0);
    ~PlaylistManager();

    QString newID();
    QString newDisplayName();

    QList<PlaylistPtr > allplaylist();
    PlaylistPtr playlist(const QString &id);

    PlaylistPtr addPlaylist(const PlaylistMeta &listinfo);
    void onCustomResort(QStringList uuids);

    void load();
    void saveSortOrder();

signals:
    void musiclistAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void musiclistRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void playlistRemove(PlaylistPtr playlist);

private:
    void insertPlaylist(const QString &id, PlaylistPtr);

private:
    QScopedPointer<PlaylistManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PlaylistManager)
};

