#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QFile>
#include <QtDebug>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QFile>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTime>
#include <QVector>
#include <QtSql>
#include "login.h"
#include <QMessageBox>
#include "mylistwidget.h"
#include "songspider.h"
#include <QRegExp>


namespace Ui {
class MusicPlayer;
}

struct songInfo
{
    int songID;
    QString songName;
    QString artistName;
    QString songUrl;
    QString picUrl;
    QString lrcUrl;
};

struct currentUserInfo{
    QString nickName;
    QString userName;
    QString password;
    QString likeMusicIndex;
    QStringList likeIndexList;
};

struct songAndPlayList{
    myListWidget * songList;
    QMediaPlaylist * playList;
};

class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

private:
    Ui::MusicPlayer *ui;
    enum playMode {random, loop, single};                                       //播放模式
    playMode currentMode;                                                       //播放模式
    QMediaPlayer * musicPlayer;                                                 //音乐播放器
    songAndPlayList * localList;                                                //本地哥曲列表
    songAndPlayList * likeList;                                                 //喜爱音乐列表
    songAndPlayList * currentList;                                              //当前的播放列表
    QVector<songInfo *> localSongList;                                          //临时存放歌曲信息的容器
    QSqlDatabase db;                                                            //连接的数据库
    int songInDbCount;                                                          //记录当前数据库的歌曲数量
    bool isLogin;                                                               //是否在登陆状态
    login loginDialog;                                                          //登录窗口
    currentUserInfo * userInfo;                                                 //当前登录用户的信息
    songSpider * spider;                                                        //爬虫类
    QString downloadPath;                                                       //下载存放路径
    int volumeBeforeMute;                                                       //静音前的音量
    QStringList songLrc;                                                        //存歌词
    QVector<double> songLrcTime;                                                //存歌词时间
    int currentLrcLine;

    void setSliderValue(int value);
    void loadLrcFile(QString filePath);
    void updateLrc(double time);
    void parseJson(QByteArray htmlStr);
    QString getIdFromDb(QString songName);
    void paintMenu(myListWidget *);
    void dealActionFromList(myListWidget *, myListWidget::actionType type);
    void loginStatusChanged(bool logined);
    void readUserInfo(QString userName);
    bool addToLocalPlayList(songInfo *, bool);
    void initDb();
    void init();
    void connectSlot();
public slots:
    void switchLocalWidget();
    void switchLikeWidget();
    void updateSlider(qint64);
    void playMediaChanged();
    void loginBtnClicked();
    void playmusic();

private slots:
    void on_addLocalMusicBtn_clicked();
    void on_searchBtn_2_clicked();
    void on_playMusic_clicked();
    void on_localMusicList_itemDoubleClicked(QListWidgetItem *item);
    void on_pause_clicked(bool checked);
    void on_prevSong_clicked();
    void on_nextSong_clicked();

    void on_likeList_itemDoubleClicked(QListWidgetItem *item);
    void on_logOutBtn_clicked();

    void on_horizontalSlider_sliderReleased();
    void on_random_clicked();
    void on_loop_clicked();
    void on_single_clicked();
    void on_searchBtn_clicked();
    void on_downloadPath_clicked();
    void on_muteBtn_clicked();
    void on_unMuteBtn_clicked();
    void on_volumeSilder_valueChanged(int value);
};

#endif // MUSICPLAYER_H
