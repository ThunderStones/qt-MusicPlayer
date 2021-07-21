#ifndef SONGSPIDER_H
#define SONGSPIDER_H


#include <QtNetwork>
#include <QDomDocument>
class songSpider: public QObject
{
    Q_OBJECT
public:
    songSpider();

    QByteArray sendHTTPPost(QString keyWord);
    QString downloadSong(const QString &id, const QString &path, const QString &songName);
    QString downloadLrc(const QString &id, const QString &path, const QString &songName);
    QString downloadPic(const QString &id, const QString &path, const QString &songName);
private:
    QNetworkAccessManager * manager;


signals:
    void sendResult(QString);
};

#endif // SONGSPIDER_H
