#ifndef SONGSPIDER_H
#define SONGSPIDER_H


#include <QtNetwork>
#include <QDomDocument>
class songSpider: public QObject
{
    Q_OBJECT
public:
    enum sourse {netEase, kugou};

    songSpider();
    QStringList * getLogInfo();

    QByteArray sendHTTPRequest(QString keyWord, sourse sourseType);
    QString downloadNetSong(const QString &id, const QString &path, const QString &songName);
    QString downloadNetLrc(const QString &id, const QString &path, const QString &songName);
    QString downloadNetPic(const QString &id, const QString &path, const QString &songName);
    void downloadNetSongList(const QString &id, const QString &path, const QString &songName);


    QStringList getKugouSongInfo(const QString &hashValue, const QString &albumId);
    QString downloadKugouSong(const QString &url, const QString &path, const QString songName);
    QString downloadKugouPic(const QString &url, const QString &path, const QString &songName);
//    QString downloadKugouSongList(const QString &hashValue, const QString &path, const QString &songName);
private:
    QNetworkAccessManager * manager;
    QStringList * logInfo;

signals:
    void sendResult(QString);
};

#endif // SONGSPIDER_H
