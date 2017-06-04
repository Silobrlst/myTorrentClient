#ifndef COMMONS_H
#define COMMONS_H

#include <QString>

#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QJsonDocument>
#include <qfile.h>
#include <qfiledialog.h>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/settings.hpp"
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_status.hpp>
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/magnet_uri.hpp"

using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;

enum TorrentContentPriority{
    TorrentContentNoLoad,
    TorrentContentNormal,
    TorrentContentHigh,
    TorrentContentMaximum
};

enum TorrentContenTreeColumns{
    TorrentContenTreeName,
    TorrentContenTreeSize,
    TorrentContenTreePriority,
    TorrentContenTreeProgress
};

enum TorrentsTableColumns{
    TorrentsTableName,
    TorrentsTableSize,
    TorrentsTableCompleted,
    TorrentsTableStatus,
    TorrentsTableSeeds,
    TorrentsTablePeers,
    TorrentsTableInputSpeed,
    TorrentsTableOutputSpeed,
    TorrentsTableBeforeCompletion,
    TorrentsTableCoef,
    TorrentsTableDate
};

int load_file(std::string const& filename, std::vector<char>& v, libtorrent::error_code& ec, int limit = 8000000);

QString unitBytes(int64_t bytesNumIn);

QString torrentsTableColumnName(int columnIn);

QString torrentStateName(torrent_status::state_t stateIn);

void saveJSON(QString fileNameIn, QJsonObject jsonIn);

QJsonObject loadJSON(QString fileNameIn);

#endif // COMMONS_H
