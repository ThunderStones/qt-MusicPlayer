#include "musicplayer.h"
#include <QApplication>

//#include <Python.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MusicPlayer w;
    // 读取设置样式表
    QFile * qss = new QFile(":/qssStyle.qss");
    if (qss->open(QIODevice::ReadOnly)) {
        qDebug() << "Open Qss Successful";
        QString qssStrint = qss->readAll();
        a.setStyleSheet(qssStrint);
    }
    w.show();
    qDebug()<<QSslSocket::sslLibraryBuildVersionString();
    qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();


    return a.exec();
}
