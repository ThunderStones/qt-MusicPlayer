#include "songspider.h"
#include <QFile>
songSpider::songSpider()
{
    manager = new QNetworkAccessManager;

}

/*
 *发送http的get请求，使用的是网易云音乐的搜索api,函数的返回值是标准格式的Json数据
 *api:http://music.163.com/api/search/get/web?csrf_token=hlpretag=&hlposttag=&s=关键字&type=1&offset=0&total=true&limit=20
 */
QByteArray songSpider::sendHTTPPost(QString keyWord)
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
 */
QString songSpider::downloadSong(const QString &id, const QString &path, const QString &songName)
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

QString songSpider::downloadLrc(const QString &id, const QString &path, const QString &songName)
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

QString songSpider::downloadPic(const QString &id, const QString &path, const QString &songName)
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
