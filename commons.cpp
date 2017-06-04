#include "commons.h"
#include "QMessageBox"

int load_file(std::string const& filename, std::vector<char>& v, libtorrent::error_code& ec, int limit){
    ec.clear();
    FILE* f = fopen(filename.c_str(), "rb");
    if (f == NULL)
    {
        ec.assign(errno, boost::system::system_category());
        return -1;
    }

    int r = fseek(f, 0, SEEK_END);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }
    long s = ftell(f);
    if (s < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    if (s > limit)
    {
        fclose(f);
        return -2;
    }

    r = fseek(f, 0, SEEK_SET);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    v.resize(s);
    if (s == 0)
    {
        fclose(f);
        return 0;
    }

    r = fread(&v[0], 1, v.size(), f);
    if (r < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    fclose(f);

    if (r != s) return -3;

    return 0;
}

QString unitBytes(int64_t bytesNumIn){
    QString ret;
    int unitNum = 0;
    float bytes = bytesNumIn;

    while(bytes > 999){
        bytes /= 1024;
        unitNum++;
    }

    ret += QString::number(bytes).left(4);
    if(ret[3] == '.'){
        ret = ret.left(3);
    }

    switch (unitNum) {
    case 0:
        ret += " Б";
        break;
    case 1:
        ret += " кБ";
        break;
    case 2:
        ret += " МБ";
        break;
    case 3:
        ret += " ГБ";
        break;
    case 4:
        ret += " ТБ";
        break;
    default:
        break;
    }

    return ret;
}

QString torrentsTableColumnName(int columnIn){
    switch(columnIn){
    case TorrentsTableName:
        return "имя";
    case TorrentsTableSize:
        return "размер";
    case TorrentsTableCompleted:
        return "завершено";
    case TorrentsTableStatus:
        return "статус";
    case TorrentsTableSeeds:
        return "сиды";
    case TorrentsTablePeers:
        return "пиры";
    case TorrentsTableInputSpeed:
        return "прием";
    case TorrentsTableOutputSpeed:
        return "отдача";
    case TorrentsTableBeforeCompletion:
        return "до завершения";
    case TorrentsTableCoef:
        return "коэфф.";
    case TorrentsTableDate:
        return "добавлен";
    }

    return "";
}

QString torrentStateName(torrent_status::state_t stateIn){
    switch(stateIn){
    case torrent_status::state_t::queued_for_checking:
        return "в очереди";
        break;
    case torrent_status::state_t::checking_files:
        return "проверка";
        break;
    case torrent_status::state_t::downloading_metadata:
        return "метаданные";
        break;
    case torrent_status::state_t::downloading:
        return "загрузка";
        break;
    case torrent_status::state_t::finished:
        return "завершен";
        break;
    case torrent_status::state_t::seeding:
        return "раздача";
        break;
    case torrent_status::state_t::allocating:
        return "размещение";
        break;
    case torrent_status::state_t::checking_resume_data:
        return "проверка";
        break;
    }

    return "";
}

void saveJSON(QString fileNameIn, QJsonObject jsonIn){
    QFile saveFile(fileNameIn);

    saveFile.open(QIODevice::WriteOnly);

    QJsonDocument doc(jsonIn);
    saveFile.write(doc.toJson());

    saveFile.close();
}

QJsonObject loadJSON(QString fileNameIn){
    QFile loadFile(fileNameIn);

    loadFile.open(QIODevice::ReadOnly);

    QByteArray data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    loadFile.close();

    return loadDoc.object();
}
