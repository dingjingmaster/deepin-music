/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlist.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "mediadatabase.h"

Playlist::Playlist(const PlaylistMeta &musiclistinfo, QObject *parent)
    : QObject(parent)
{
    listmeta = musiclistinfo;
    m_history = listmeta.musicIds;
}

int Playlist::length()
{
    return listmeta.musicMap.size();
}

const MusicMeta Playlist::first()
{
    return listmeta.musicMap.value(listmeta.musicIds.value(0));
}

const MusicMeta Playlist::prev(const MusicMeta &info)
{
    if (0 == listmeta.musicIds.length()) {
        return MusicMeta();
    }
    auto index = listmeta.musicIds.indexOf(info.hash);
    auto prev = (index + listmeta.musicIds.length() - 1) % listmeta.musicIds.length();
    return listmeta.musicMap.value(listmeta.musicIds.at(prev));
}

const MusicMeta Playlist::next(const MusicMeta &info)
{
    if (0 == listmeta.musicIds.length()) {
        return MusicMeta();
    }
    auto index = listmeta.musicIds.indexOf(info.hash);
    auto prev = (index + 1) % listmeta.musicIds.length();
    return listmeta.musicMap.value(listmeta.musicIds.at(prev));
}

const MusicMeta Playlist::music(int index)
{
    return listmeta.musicMap.value(listmeta.musicIds.value(index));
}

const MusicMeta Playlist::music(const QString &id)
{
    return listmeta.musicMap.value(id);
}

bool Playlist::isLast(const MusicMeta &info)
{
    return listmeta.musicIds.last() == info.hash;
}

bool Playlist::contains(const MusicMeta &info)
{
    return listmeta.musicMap.contains(info.hash);
}

QString Playlist::id()
{
    return listmeta.uuid;
}

QString Playlist::displayName()
{
    return listmeta.displayName;
}

QString Playlist::icon()
{
    return listmeta.icon;
}

bool Playlist::readonly()
{
    return listmeta.readonly;
}

bool Playlist::editmode()
{
    return listmeta.editmode;
}

bool Playlist::hide()
{
    return listmeta.hide;
}

MusicList Playlist::allmusic()
{
    MusicList mlist;
    for (auto id : listmeta.musicIds) {
        mlist << listmeta.musicMap.value(id);
    }
    return mlist;
}

void Playlist::buildHistory(const QString &last)
{
    auto lastindex = listmeta.musicIds.indexOf(last);
    m_history.clear();
    for (int i = lastindex + 1; i < listmeta.musicIds.length(); ++i) {
        m_history.append(listmeta.musicIds.value(i));
    }
    for (int i = 0; i <= lastindex; ++i) {
        m_history.append(listmeta.musicIds.value(i));
    }
}

void Playlist::clearHistory()
{
    m_history.clear();
}

void Playlist::load()
{
    QList<PlaylistMeta> list;
    QSqlQuery query;
    query.prepare(QString("SELECT music_id FROM playlist_%1").arg(listmeta.uuid));

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QStringList musicIDs;
    while (query.next()) {
        auto musicID = query.value(0).toString();
        listmeta.musicIds << musicID;
        musicIDs << QString("\"%1\"").arg(musicID);
    }
    auto sqlStr = QString("SELECT hash, localpath, title, artist, album, "
                          "filetype, length, size "
                          "FROM music WHERE hash IN (%1)").arg(musicIDs.join(","));
    if (!query.exec(sqlStr)) {
        qWarning() << query.lastError();
        return;
    }

    while (query.next()) {
        MusicMeta info;
        info.hash = query.value(0).toString();
        info.localpath = query.value(1).toString();
        info.title = query.value(2).toString();
        info.artist = query.value(3).toString();
        info.album = query.value(4).toString();
        info.filetype = query.value(5).toString();
        info.length = query.value(6).toInt();
        info.size = query.value(6).toInt();
        listmeta.musicMap.insert(info.hash, info);
    }
}

void Playlist::setDisplayName(const QString &name)
{
    listmeta.displayName = name;

    QSqlQuery query;
    query.prepare("UPDATE playlist SET displayname = :displayname WHERE uuid= :uuid");
    query.bindValue(":displayname", listmeta.displayName);
    query.bindValue(":uuid", listmeta.uuid);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }
}

void Playlist::appendMusic(const MusicMeta &meta)
{
    if (listmeta.musicMap.contains(meta.hash)) {
        qDebug() << "add dump music " << meta.hash << meta.localpath;
        return;
    }

    listmeta.musicIds << meta.hash;
    listmeta.musicMap.insert(meta.hash, meta);

    MediaDatabase::insertMusic(meta, listmeta);

    emit musicAdded(meta);
}

void Playlist::removeMusic(const MusicMeta &info)
{
    if (info.hash.isEmpty()) {
        qCritical() << "Cannot remove empty id";
        return;
    }
    if (!listmeta.musicMap.contains(info.hash)) {
        qWarning() << "no such id in playlist" << info.hash << info.localpath << listmeta.displayName;
        return;
    }

    m_history.removeAll(info.hash);
    listmeta.musicIds.removeAll(info.hash);
    listmeta.musicMap.remove(info.hash);

    emit musicRemoved(info);

    MediaDatabase::deleteMusic(info, listmeta);
}