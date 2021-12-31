#include "songspider.h"
#include <QFile>
songSpider::songSpider()
{
    manager = new QNetworkAccessManager;
    logInfo = new QStringList;
}

QStringList * songSpider::getLogInfo()
{
    return logInfo;
}

/*
 * sourse用于区分资源来源于网易云音乐或酷狗
 * 发送http的get请求，使用的是网易云音乐的搜索api,函数的返回值是标准格式的Json数据
 * api:http://music.163.com/api/search/get/web?csrf_token=hlpretag=&hlposttag=&s=关键字&type=1&offset=0&total=true&limit=20
 * api:http://msearchcdn.kugou.com/api/v3/search/song?showtype=14&highlight=em&pagesize=30&tag_aggr=1&tagtype=全部&plat=0&sver=5&keyword=关键字&correct=1&api_ver=1&version=9108&page=1&area_code=1&tag=1&with_res_tag=1
 */
QByteArray songSpider::sendHTTPRequest(QString keyWord, sourse sourseType)
{
    switch (sourseType) {
    case netEase:
    {
    qDebug() << "send post";
    QNetworkRequest * request = new QNetworkRequest
                          (QUrl(QString("http://music.163.com/api/search/get/web?csrf_token=hlpretag=&hlposttag=&s=%1"
                          "&type=1&offset=0&total=true&limit=20")
                          .arg(keyWord)));

    QNetworkReply * reply = manager->get(*request);
    //创建计时器等待请求结束
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QString jsonStr(reply->readAll());
    //由于网易云音乐的api返回的Json数据不规范，一个Json对象里有两个同名key（alias）,
    //利用正则表达式除去所有的alias
    QRegExp rx("\"alias\":\\[.*\\],");
    rx.setMinimal(true);
    jsonStr.remove(rx);

    return jsonStr.toUtf8();
    }
    case kugou:
    {
        QString url = QString("http://msearchcdn.kugou.com/api/v3/search/song?"
                              "showtype=14&"
                              "highlight=em&"
                              "pagesize=30&"
                              "tag_aggr=1&"
                              "tagtype=全部&"
                              "plat=0&sver=5&"
                              "keyword=%1&"
                              "correct=1&"
                              "api_ver=1&"
                              "version=9108&"
                              "page=1&"
                              "area_code=1&"
                              "tag=1&"
                              "with_res_tag=1").arg(keyWord);
        QNetworkRequest * request = new QNetworkRequest(url);

        QNetworkReply * reply = manager->get(*request);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();

//        QFile f1(QString("searchresult.json"));
//        f1.open(QFile::WriteOnly);
//        f1.write(reply->readAll());
//        f1.close();
        QString data = reply->readAll();
        data.remove("<!--KG_TAG_RES_START-->").remove("<!--KG_TAG_RES_END-->");
        return data.toUtf8();
    }
    }
}

/* 以下三个函数：
 * 经过数据分析，猜测在网易云音乐的数据库中有id字段为数据库的主键，由id值可以获取歌曲以及歌词等其他信息
 * 获取歌曲api：https://music.163.com/song/media/outer/url?id=(id值).mp3
 * 获取歌词api：https://music.163.com/api/song/lyric?id=(id值)&lv=1&kv=1&tv=-1
 * 获取专辑图片api：http://music.163.com/api/song/detail/?id=(id值)&ids=%5B(id值)%5D"
 * 访问歌曲api时，服务器会将网页重定向到真正的资源地址，资源地址保存在响应头的Location中，因此需要再次请求
 * 参数：
 * id：歌曲在网易云数据库中的id
 * path：保存路径
 * songName：歌曲名
 * 如果运行时跳出SSL初始化错误问题，需要将Qt安装路径下编译器的bin文件夹内的ssleay32.dll和libeay32.dll文件赋值到项目的生成文件夹中
 */
QString songSpider::downloadNetSong(const QString &id, const QString &path, const QString &songName)
{
    QNetworkRequest * req = new QNetworkRequest(QUrl(QString("https://music.163.com/song/media/outer/url?id=%1.mp3").arg(id)));
    QNetworkReply * reply = manager->get(*req);

    //创建计时器等待服务器响应结束
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QUrl songDownloadUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();

    delete req;
    req = new QNetworkRequest(songDownloadUrl);
    reply = manager->get(*req);
    QEventLoop eventLoop1;
    connect(reply, &QNetworkReply::finished, &eventLoop1, &QEventLoop::quit);
    eventLoop1.exec();

    QString fileName = path + "/" + songName + ".mp3";
    QFile * fileWriter = new QFile(fileName);
    fileWriter->open(QFile::WriteOnly);
    fileWriter->write(reply->readAll());
    fileWriter->close();
    //qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    return fileName;
}

QString songSpider::downloadNetLrc(const QString &id, const QString &path, const QString &songName)
{
    QNetworkRequest req(QUrl(QString("https://music.163.com/api/song/lyric?id=%1&lv=1&kv=1&tv=-1").arg(id)));
    QString filePath = path + "/" + songName + ".lrc";
    QNetworkReply * reply = manager->get(req);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    //解析Json获得歌词
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
    if (doc.isNull() || jsonError.error != QJsonParseError::NoError) {
        qDebug() << "parser json error" << jsonError.error;
        return "JsonError";
    }
    QString lrc = doc.object()
                     .value("lrc")
                     .toObject()
                     .value("lyric")
                     .toString();

    QFile * lrcWriter = new QFile(filePath);
    lrcWriter->open(QFile::WriteOnly);
    lrcWriter->write(lrc.toUtf8());
    lrcWriter->close();

    return filePath;
}

QString songSpider::downloadNetPic(const QString &id, const QString &path, const QString &songName)
{
    //获取专辑图片Url
    QNetworkRequest req1(QUrl(QString("http://music.163.com/api/song/detail/?id=" + id + "&ids=%5B"+ id +"%5D")));
    QString filePath = path + "/" + songName + ".jpg";
    QNetworkReply * reply1 = manager->get(req1);

    QEventLoop eventLoop1;
    connect(reply1, &QNetworkReply::finished, &eventLoop1, &QEventLoop::quit);
    eventLoop1.exec();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(reply1->readAll(), &jsonError);
    if (doc.isNull() || jsonError.error != QJsonParseError::NoError) {
        qDebug() << "parser json error" << jsonError.error;
        return "JsonError";
    }
    QString picUrl = doc.object()
                        .value("songs")
                        .toArray()
                        .at(0)
                        .toObject()
                        .value("album")
                        .toObject()
                        .value("picUrl")
                        .toString();

    qDebug() << picUrl;
    //下载专辑图片
    QNetworkRequest req2(picUrl);
    QNetworkReply * reply2 = manager->get(req2);

    QEventLoop eventLoop2;
    connect(reply2, &QNetworkReply::finished, &eventLoop2, &QEventLoop::quit);
    eventLoop2.exec();

    qDebug() << reply2->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QFile * picWriter = new QFile(filePath);
    picWriter->open(QFile::WriteOnly);
    picWriter->write(reply2->readAll());
    picWriter->close();
    qDebug() << "download picture complete";
    return filePath;
}

void songSpider::downloadNetSongList(const QString &id, const QString &path, const QString &songName)
{

}

QStringList songSpider::getKugouSongInfo(const QString &hashValue, const QString &albumId)
{
    QString url = QString("https://www.kugou.com/yy/index.php?r=play/getdata&hash=%1&album_id=%2").arg(hashValue, albumId);
    QNetworkRequest * request = new QNetworkRequest(url);
    qDebug() << url;
    request->setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36");
    qDebug();
    QNetworkReply * reply = manager->get(*request);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
    if (doc.isNull() || jsonError.error != QJsonParseError::NoError) {
        qDebug() << "parser json error" << jsonError.error;
    }
    qDebug() << doc.object();
    if (doc.object().value("err_code").toInt() != 0)
        return QStringList{"error"};
    QJsonObject data = doc.object().value("data").toObject();
    QString songUrl = data.value("play_url").toString();
    QString lrc = data.value("lyrics").toString();
    QString picUrl = data.value("img").toString();

    return QStringList{songUrl, lrc, picUrl};
}

QString songSpider::downloadKugouSong(const QString &url, const QString &path, const QString songName)
{
    QNetworkRequest * request = new QNetworkRequest(url);
    QNetworkReply * reply = manager->get(*request);

    QString wholePath = path + QString("/") + songName + QString(".mp3");
    QFile writer(wholePath);
    writer.open(QFile::WriteOnly);
    writer.write(reply->readAll());

    return wholePath;
}

QString songSpider::downloadKugouPic(const QString &url, const QString &path, const QString &songName)
{
    QNetworkRequest * request = new QNetworkRequest(url);
    QNetworkReply * reply = manager->get(*request);

    QString wholePath = path + QString("/") + songName + QString(".jpg");
    QFile writer(wholePath);
    writer.open(QFile::WriteOnly);
    writer.write(reply->readAll());

    return wholePath;
}

