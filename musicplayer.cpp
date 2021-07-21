#include "musicplayer.h"
#include "ui_musicplayer.h"

MusicPlayer::MusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicPlayer),
    localList(nullptr),
    likeList(nullptr),
    loginDialog(db, this)
{
    ui->setupUi(this);
    init();
    initDb();
    connectSlot();
}

MusicPlayer::~MusicPlayer()
{
    db.close();
    musicPlayer->pause();
    delete ui;
    delete userInfo;
}

//初始化控件和成员变量
void MusicPlayer::init()
{
    //初始化成员变量
    localList = new songAndPlayList;
    likeList = new songAndPlayList;
    musicPlayer = new QMediaPlayer(this);
    localList->playList = new QMediaPlaylist(this);
    likeList->playList = new QMediaPlaylist(this);
    localList->songList = ui->localMusicList;
    likeList->songList = ui->likeList;
    downloadPath = nullptr;

    ui->volumeSilder->setMinimum(0);
    ui->volumeSilder->setMaximum(100);
    ui->volumeSilder->setValue(50);
    musicPlayer->setVolume(50);
    volumeBeforeMute = 50;
    currentLrcLine = 0;
    ui->unMuteBtn->hide();

    //设置按钮hover时的鼠标指针形状为手性
    ui->prevSong->setCursor(QCursor(Qt::PointingHandCursor));
    ui->nextSong->setCursor(QCursor(Qt::PointingHandCursor));
    ui->pause->setCursor(QCursor(Qt::PointingHandCursor));
    ui->loop->setCursor(QCursor(Qt::PointingHandCursor));
    ui->random->setCursor(QCursor(Qt::PointingHandCursor));
    ui->single->setCursor(QCursor(Qt::PointingHandCursor));
    ui->likeListBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->localListBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->searchBtn_2->setCursor(QCursor(Qt::PointingHandCursor));
    ui->searchBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->addLocalMusicBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->playMusic->setCursor(QCursor(Qt::PointingHandCursor));
    ui->loginBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->logOutBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->albumPic->setScaledContents(true);
    ui->lrcLabel->clear();

    this->setFixedSize(QSize(1280, 720));
    ui->stackedWidget->setCurrentWidget(ui->searchWidget);
    currentMode = random;
    currentList = localList;

    ui->logOutBtn->hide();
    ui->currentTime->setText("00:00");
    ui->totalTime->setText("00:00");
    localList->playList->setPlaybackMode(QMediaPlaylist::Random);
    likeList->playList->setPlaybackMode(QMediaPlaylist::Random);

    spider = new songSpider;

}

//初始化数据库并读取数据，添加到本地播放列表和listWidget中
void MusicPlayer::initDb()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("songInfo.db");
    db.setUserName("admin");
    db.setPassword("pwd");
    db.open();

    QSqlQuery query;
    bool success1 = query.exec("create table songInformation"
                              "(id int primary key,"
                              "songName varchar,"
                              "artistName varchar,"
                              "songUrl varchar,"
                              "picUrl varchar,"
                              "lrcUrl varchar)");
    if (success1)
        qDebug() << "make table1 successful";
    else
        qDebug() << "fail to make table1";

    bool success2 = query.exec("create table searchResult"
                               "(id varchar primary key,"
                               "songName varchar)");
    if (success2)
        qDebug() << "make table2 successful";
    else
        qDebug() << "fail to make table2";

    query.exec("select * from songInformation");
    while (query.next()) {
        songInfo * song = new songInfo;
        song->songID = query.value(0).toInt();
        song->songName = query.value(1).toString();
        song->artistName = query.value(2).toString();
        song->songUrl = query.value(3).toString();
        song->picUrl = query.value(4).toString();
        song->lrcUrl = query.value(5).toString();
        localSongList.append(song);
        addToLocalPlayList(song, false);
    }
    query.exec("SELECT MAX(id) from songInformation");
    query.next();
    songInDbCount = query.value(0).toInt() - 1;
    qDebug() << songInDbCount;


}

//若歌曲不在数据库中，则将歌曲添加到本地播放列表和listWidget中
//flag 用于区分是否判断歌曲在数据库中
bool MusicPlayer::addToLocalPlayList(songInfo * song, bool flag)
{
    QSqlQuery query;
    query.exec(QString("select songUrl from songInformation where songUrl = \"%1\"").arg(song->songUrl));
    query.next();
    if (flag && query.value(0).toString() == song->songUrl) {
        qDebug() << song->songName << " exist";
        return false;
    } else {
        //将歌曲添加到本地播放列表和本地列表控件中
        localList->playList->addMedia(QUrl::fromLocalFile(song->songUrl));
        QString songName = song->songName;
        QListWidgetItem * songItem = new QListWidgetItem(songName);
        ui->localMusicList->addItem(songItem);
        songItem->setToolTip(songName);
//        qDebug() << "向列表中添加" << song->songName;
        return true;
    }
}

void MusicPlayer::connectSlot()
{
    connect(musicPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(updateSlider(qint64)));
    connect(musicPlayer, &QMediaPlayer::metaDataAvailableChanged, this, &MusicPlayer::playMediaChanged);

    connect(ui->likeListBtn, SIGNAL(clicked()), this, SLOT(switchLikeWidget()));
    connect(ui->localListBtn, SIGNAL(clicked()), this, SLOT(switchLocalWidget()));

    connect(&loginDialog, &login::userLogin, this, &MusicPlayer::readUserInfo);
    connect(ui->loginBtn, &QPushButton::clicked, this, &MusicPlayer::loginBtnClicked);

    connect(ui->localMusicList, &myListWidget::transferSignals, this, &MusicPlayer::dealActionFromList);
    connect(ui->likeList, &myListWidget::transferSignals, this, &MusicPlayer::dealActionFromList);
    connect(ui->searchResultList, &myListWidget::transferSignals, this, &MusicPlayer::dealActionFromList);

    connect(localList->songList, &myListWidget::listRightClicked, this, &MusicPlayer::paintMenu);
    connect(likeList->songList, &myListWidget::listRightClicked, this, &MusicPlayer::paintMenu);
    connect(ui->searchResultList, &myListWidget::listRightClicked, this, &MusicPlayer::paintMenu);

    connect(ui->horizontalSlider, &mySlider::sliderClicked, this, &MusicPlayer::setSliderValue);
}

void MusicPlayer::playmusic(){}


// 切换到我喜欢页面
void MusicPlayer::switchLikeWidget()
{
    qDebug() << "switch to like widget";
    ui->stackedWidget->setCurrentWidget(ui->likeListWidget);
}

//切换到本地音乐页面
void MusicPlayer::switchLocalWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->localListWidget);
}

//切换到搜索页面
void MusicPlayer::on_searchBtn_2_clicked()
{
    qDebug() << "switch to search widget";
    ui->stackedWidget->setCurrentWidget(ui->searchWidget);
}


//将本地文件添加到列表
void MusicPlayer::on_addLocalMusicBtn_clicked()
{
    qDebug() << "call add file func";
    //获取文件名字
    QStringList fileNames = QFileDialog::getOpenFileNames(this, QString("添加本地音乐"), ".", "MP3 File(*.mp3)");



    for (int i = 0; i < fileNames.size(); ++i) {
//        QMediaContent * tempMedia = new QMediaContent(QUrl::fromLocalFile(fileNames[i]));
//        localPlayList->addMedia(*tempMedia);
        //建立歌曲信息
        QString songName = fileNames[i].split("/").last().replace(".mp3", "");
        songInfo * song = new songInfo{++songInDbCount, songName, "", fileNames[i], "", ""};
        localSongList.append(song);
        if (!addToLocalPlayList(song, true))
            continue;//数据库已存在，跳到下一循环，处理下一首歌

        //添加歌曲信息到数据库
        QSqlQuery query;
        query.prepare("insert into songInformation values(?,?,?,?,?,?)");
        query.bindValue(0, song->songID);
        query.bindValue(1, song->songName);
        query.bindValue(2, song->artistName);
        query.bindValue(3, song->songUrl);
        query.bindValue(4, song->picUrl);
        query.bindValue(5, song->lrcUrl);

        bool success = query.exec();
        if (success)
            qDebug() << QString("插入%1成功").arg(song->songName);
        else
            qDebug() << query.lastError().driverText() << " 插入失败";


//        qDebug() << songName;


    }

}


//本地音乐的播放按钮，实现音乐播放
void MusicPlayer::on_playMusic_clicked()
{
    musicPlayer->setPlaylist(localList->playList);
    currentList = localList;
    musicPlayer->play();
}

//双击喜爱列表播放音乐
void MusicPlayer::on_likeList_itemDoubleClicked(QListWidgetItem *item)
{
    int i = ui->likeList->currentRow();
    currentList = likeList;
    qDebug() << "225" << currentList->playList;
    musicPlayer->setPlaylist(likeList->playList);
    likeList->playList->setCurrentIndex(i);
    musicPlayer->play();
    ui->pause->setChecked(false);
}


//双击本地列表播放音乐
void MusicPlayer::on_localMusicList_itemDoubleClicked(QListWidgetItem *item)
{
    int i = ui->localMusicList->currentRow();
    currentList = localList;
    qDebug() << "238" << currentList->playList;
    musicPlayer->setPlaylist(localList->playList);
    localList->playList->setCurrentIndex(i);
    musicPlayer->play();
    ui->pause->setChecked(false);
}

//暂停 播放
void MusicPlayer::on_pause_clicked(bool checked)
{
    if (checked)
        musicPlayer->pause();
    else
        musicPlayer->play();
}

//上一首
void MusicPlayer::on_prevSong_clicked()
{
    qDebug() << "上一首";
    switch (currentMode) {
    case loop:
    case random:
        currentList->playList->previous();
        break;
    case single:
        currentList->playList->setPlaybackMode(QMediaPlaylist::Loop);
        currentList->playList->previous();
        currentList->playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }

    qDebug() << currentList->playList->currentIndex();
}

// 下一首
void MusicPlayer::on_nextSong_clicked()
{
    qDebug() << "下一首";
    switch (currentMode) {
    case loop:
    case random:
        currentList->playList->next();
        break;
    case single:
        currentList->playList->setPlaybackMode(QMediaPlaylist::Loop);
        currentList->playList->next();
        currentList->playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }
}

//随音乐播放更新进度条
void MusicPlayer::updateSlider(qint64 pos)
{

//    int timeLen = static_cast<int>(musicPlayer->duration() / 1000); //当前歌曲长度 单位为秒
    int position = static_cast<int>(pos / 1000);
    int hh = position / 3600;
    int hm = (position % 3600) / 60;
    int hs = position % 60;

    QTime timeNow(hh,hm,hs);
    QString currentTime = timeNow.toString("mm:ss");
    ui->currentTime->setText(currentTime);

    ui->horizontalSlider->setValue(static_cast<int>(pos));
    updateLrc(static_cast<double>((pos / 1000)));
}

//播放的歌曲改变，更新播放音乐的信息和进度条
void MusicPlayer::playMediaChanged()
{
    int timeTotalLen = (musicPlayer->metaData("Duration")).toInt();
//    qDebug() << "调用playMediaChanged" << timeTotalLen;
    if (timeTotalLen == 0) {
//        QMessageBox::information(this, "无法解析", "缺少解码器，无法解析该文件。");
//        ui->songName->setText(QString("无法解析"));
        return;
    }
    int timeTotalInt = static_cast<int>(timeTotalLen / 1000);
    QMediaContent currentMedia = currentList->playList->currentMedia();

    //获取歌名



//    int timeTotalLen = static_cast<int>(musicPlayer->duration() / 1000); //当前歌曲长度 单位为秒
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(timeTotalLen);
    int hh = timeTotalInt / 3600;
    int hm = (timeTotalInt % 3600) / 60;
    int hs = timeTotalInt % 60;
    QTime timeTotal(hh,hm,hs);
    QString totalTime = timeTotal.toString("mm:ss");


    QSqlQuery query;
    query.exec(QString("select songName, picUrl, lrcUrl from songInformation where songUrl = \"%1\"").arg(currentMedia.canonicalUrl().toString().remove("file:///")));
    query.next();

    QString songName = query.value(0).toString();
    qDebug() << songName;

    ui->totalTime->setText(totalTime);
    ui->songName->setText(songName);

    QString picUrl = query.value(1).toString();
    QString lrcUrl = query.value(2).toString();
    if (picUrl.isEmpty())
        ui->albumPic->setPixmap(QPixmap(":/image/defaultAlbumPic.png"));
    else {
        QImage pic(picUrl);
        pic.scaled(101, 101);
        ui->albumPic->setPixmap(QPixmap::fromImage(pic));
    }

    if (lrcUrl.isEmpty()) {
        ui->lrcLabel->setText("未找到歌词");
        songLrc.clear();
        songLrcTime.clear();
    } else {
        loadLrcFile(lrcUrl);
    }

}


// 登录状态改变
void MusicPlayer::loginStatusChanged(bool logined)
{
    if (logined) {
        ui->loginBtn->hide();
        ui->logOutBtn->show();
        ui->userNickName->setText(userInfo->nickName);
    } else {
        ui->loginBtn->show();
        ui->logOutBtn->hide();
        ui->userNickName->setText("请登录");
        ui->likeList->clear();
        likeList->playList->clear();
        currentList = localList;
            qDebug() << "354" << currentList->playList;
    }
}


//从数据库读取用户信息 添加用户喜爱音乐
void MusicPlayer::readUserInfo(QString userName)
{

    // 读取用户信息
    QSqlQuery query;
    query.exec(QString("SELECT * from userInfo where userName = \"%1\"").arg(userName));
    query.next();
    if (!query.isValid()) {
        QMessageBox::information(this, "错误", "读取用户信息失败");
        return;
    }

    userInfo = new currentUserInfo{
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(3).toString().split(",")
    };


    //将喜爱歌曲添加到喜爱列表和播放列表
    QStringList index = userInfo->likeIndexList;
    index.removeFirst();
    for (int i = 0; i < index.size(); i++) {
        query.exec(QString("select songName, artistName, songUrl from songInformation where id = %1").arg(index[i].toInt()));
        query.next();
        if (!query.isValid()) {
            qDebug() << "fail to add to likeList";
            continue;
        } else {
            ui->likeList->addItem(query.value(0).toString());
            qDebug() << query.value(2).toString();
            likeList->playList->addMedia(QUrl::fromLocalFile(query.value(2).toString()));
        }
    }

    isLogin = true;
    loginStatusChanged(isLogin);

}

// 注销
void MusicPlayer::on_logOutBtn_clicked()
{
    isLogin = false;
    loginStatusChanged(false);
}

//按下登录按钮
void MusicPlayer::loginBtnClicked()
{
    qDebug() << "login";
    loginDialog.exec();
}

//拖动进度条 定位歌曲播放位置
void MusicPlayer::on_horizontalSlider_sliderReleased()
{
    musicPlayer->setPosition(ui->horizontalSlider->value());
}

//处理列表右键信号
void MusicPlayer::dealActionFromList(myListWidget * list, myListWidget::actionType type)
{

//    qDebug() << "423" << currentList->songList;
    switch (type) {
    case myListWidget::actPlay:
    {
        currentList = list == localList->songList ? localList : likeList; //网络 歌曲应下载后播放
        qDebug() << "action play";

        musicPlayer->setPlaylist(currentList->playList);
        currentList->playList->setCurrentIndex(list->currentRow());
        ui->songName->setText(currentList->songList->currentItem()->text());
        qDebug() << list->currentRow();
        if (ui->pause->isChecked())
            ui->pause->setChecked(false);

        musicPlayer->play();
        break;
    }
    case myListWidget::actLike:
    {
        if (!isLogin) {
            QMessageBox::information(this, "未登录", "请先登录。");
            return;
        }
        qDebug() << localList->songList->currentRow();
        QString id = getIdFromDb(localList->songList->currentItem()->text());
        qDebug() << QString("").split(",") << (QStringList()<<"").join(",");
        if (userInfo->likeIndexList.contains(id)) {
            QMessageBox::information(this, "提示", "这首歌已经在喜爱列表中。");
            return;
        }
        userInfo->likeIndexList.append(id);
        userInfo->likeMusicIndex = userInfo->likeIndexList.join(",");
        QSqlQuery query;
        if (!query.exec(QString("update userInfo set likeMusicIndex = \"%1\" where userName = \"%2\"")
                        .arg(userInfo->likeMusicIndex, userInfo->userName))) {
            QMessageBox::information(this, "失败", "添加失败。");
            return;
        }
        qDebug() << localList->songList->currentItem()->text();
        likeList->songList->addItem(localList->songList->currentItem()->text());
        //获取路径
        query.exec(QString("select songUrl from songInformation where id = %1").arg(getIdFromDb(localList->songList->currentItem()->text())));
        query.next();

        likeList->playList->addMedia(QUrl::fromLocalFile(query.value(0).toString()));
        break;
    }

    case myListWidget::actUnlike:
    {
        int row = likeList->songList->currentRow();
        likeList->playList->removeMedia(row);
        QListWidgetItem * song = likeList->songList->takeItem(row);
        //用户数据和数据库更新

        QSqlQuery query;
        QString id = getIdFromDb(song->text());
        userInfo->likeIndexList.removeOne(id);
        userInfo->likeMusicIndex = userInfo->likeIndexList.join(",");
        qDebug() << userInfo->likeIndexList.join(",") << id << song->text();
        query.exec(QString("update userInfo set likeMusicIndex = \"%1\" where userName = \"%2\"")
                   .arg(userInfo->likeMusicIndex, userInfo->userName));
        QMessageBox::information(this, "成功", "从我喜欢的列表中移除成功");
        break;
    }


    case myListWidget::actDelete:
    {
        if (QMessageBox::question(this, "确认删除", "从本地列表移除后，同时也会从喜爱歌曲中移除，是否继续？",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return;
        //从播放列表中删除
        localList->playList->removeMedia(localList->songList->currentRow());

        //从列表中删除
        QListWidgetItem * song = localList->songList->takeItem(localList->songList->currentRow());
        QString songName = song->text();
        int i = 0;
        if (isLogin) {
            for (i = 0; i < likeList->songList->count(); i++) {
                if (likeList->songList->item(i)->text() == songName) {
                    likeList->songList->takeItem(i);
                    qDebug() << i;
                    likeList->playList->removeMedia(i);
                    break;
                }
            }
        }
        QSqlQuery query;
        query.exec(QString("select id from songInformation where songName = \"%1\"").arg(songName));
        query.next();
        int id = query.value(0).toInt();
        if (isLogin && i != likeList->songList->count()) {
            userInfo->likeIndexList.removeOne(QString::number(id));
            userInfo->likeMusicIndex = userInfo->likeIndexList.join(",");
            query.exec(QString("update userInfo set likeMusicIndex = \"%1\" where userName = \"%2\"")
                       .arg(userInfo->likeMusicIndex, userInfo->userName));
        }
        bool success = query.exec(QString("delete from songInformation where id = %1").arg(id));
        if (success)
            QMessageBox::information(this, "成功", "删除成功");
        else
            qDebug() << "删除失败";

        break;
    }
    case myListWidget::actDownload:
    {
        QString songName = ui->searchResultList->currentItem()->text();
        QSqlQuery query;
        query.exec(QString("select id from searchResult where songName = \"%1\"").arg(songName));
        query.next();
        QString songId = query.value(0).toString();

        if (downloadPath == nullptr)
            on_downloadPath_clicked();
        QString songUrl = spider->downloadSong(songId, downloadPath, songName);
        QString lrcUrl = spider->downloadLrc(songId, downloadPath, songName);
        QString picUrl = spider->downloadPic(songId, downloadPath, songName);
        query.exec(QString("select songUrl from songInformation where songName = \"%1\"").arg(songName));
        query.next();
        if (query.isValid()) {
            QMessageBox::information(this, "提示", "该歌曲已下载");
            return;
        }
        songInfo * song = new songInfo{++songInDbCount, songName, "", songUrl, picUrl, lrcUrl};
        addToLocalPlayList(song, false);
        QMessageBox::information(this, "提示", "下载成功，已添加到本地列表。");
        qDebug() << query.exec(QString("insert into songInformation values(\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\")")
                      .arg(QString::number(++songInDbCount), songName, "", songUrl, picUrl, lrcUrl));

        break;
    }
    }
}



//绘制列表右键菜单
void MusicPlayer::paintMenu(myListWidget * list)
{
    if (list == localList->songList) {
        list->popMenu->addAction(list->play);
        list->popMenu->addAction(list->like);
        list->popMenu->addAction(list->deleteSong);
        list->popMenu->exec(QCursor::pos());
    } else if (list == likeList->songList) {
        list->popMenu->addAction(list->play);
        list->popMenu->addAction(list->unlike);
        list->popMenu->exec(QCursor::pos());
    } else if (list == ui->searchResultList) {
        list->popMenu->addAction(list->download);
        list->popMenu->exec(QCursor::pos());
    }
}

//设置播放模式 为随机播放
void MusicPlayer::on_random_clicked()
{
    currentMode = random;
    localList->playList->setPlaybackMode(QMediaPlaylist::Random);
    likeList->playList->setPlaybackMode(QMediaPlaylist::Random);
}

//设置播放模式 为顺序播放
void MusicPlayer::on_loop_clicked()
{
    currentMode = loop;
    localList->playList->setPlaybackMode(QMediaPlaylist::Loop);
    likeList->playList->setPlaybackMode(QMediaPlaylist::Loop);
}

//设置播放模式 为单曲循环
void MusicPlayer::on_single_clicked()
{
    currentMode = single;
    localList->playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    likeList->playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
}

//根据歌名从数据库中获取id
QString MusicPlayer::getIdFromDb(QString songName)
{
    QSqlQuery query;
    query.exec(QString("select id from songInformation where songName = \"%1\"").arg(songName));
    query.next();
    return query.value(0).toString();
}

//解析get到的Json数据并存到数据库中，
void MusicPlayer::parseJson(QByteArray jsonData)
{

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &jsonError);
    if (doc.isNull() || jsonError.error != QJsonParseError::NoError) {
        qDebug() << "parse Json Error" << jsonError.error;
        return;
    }

    ui->searchResultList->clear();

    QSqlQuery query;
    query.exec("delete from searchResult");
    QJsonArray songInfoArray = doc.object().value("result").toObject().value("songs").toArray();
    if (songInfoArray.size() == 0){
        QMessageBox::information(this, "提示", "未搜索到相关歌曲。");
        return;
    }
    for (int i = 0; i < songInfoArray.size(); i++) {
        int feeFlag = songInfoArray.at(i).toObject().value("fee").toInt();
        if (feeFlag != 0 && feeFlag != 8)
            continue;
        QString id = songInfoArray.at(i)
                                  .toObject()
                                  .value("id")
                                  .toVariant()
                                  .toString();

        QString songName = songInfoArray.at(i)
                                        .toObject()
                                        .value("name")
                                        .toString()
                                        + "-" +
                           songInfoArray.at(i)
                                        .toObject()
                                        .value("artists")
                                        .toArray()
                                        .at(0)
                                        .toObject()
                                        .value("name")
                                        .toString();
        qDebug() << id <<songName;
        query.exec(QString("insert into searchResult values(\"%1\",\"%2\")").arg(id, songName));

        ui->searchResultList->addItem(songName);
    }


}

//点击搜索按钮
void MusicPlayer::on_searchBtn_clicked()
{
    if (ui->searchText->text().isEmpty()) {
        QMessageBox::information(this, "提示", "请输入关键字");
        return;
    }
    QByteArray res = spider->sendHTTPPost(ui->searchText->text());
    parseJson(res);

}

//点击设置路径按钮，获得下载路径
void MusicPlayer::on_downloadPath_clicked()
{
    QUrl * path = new QUrl;
    path->setPath(QFileDialog::getExistingDirectory(this, "选择下载路径"));
    downloadPath = path->path();
}




//按下静音按钮
void MusicPlayer::on_muteBtn_clicked()
{
    ui->muteBtn->hide();
    ui->unMuteBtn->show();
    volumeBeforeMute = ui->volumeSilder->value();
    ui->volumeSilder->setValue(0);
    musicPlayer->setMuted(true);
}

//按下取消静音按钮
void MusicPlayer::on_unMuteBtn_clicked()
{
    ui->muteBtn->show();
    ui->unMuteBtn->hide();
    ui->volumeSilder->setValue(volumeBeforeMute);
    musicPlayer->setMuted(false);

}

//加载歌词文件
void MusicPlayer::loadLrcFile(QString filePath)
{
    songLrc.clear();
    songLrcTime.clear();
    QFile reader(filePath);
    reader.open(QFile::ReadOnly | QIODevice::Text);
    QRegExp re("\\[(\\d+):(\\d+).(\\d+)");
    while (!reader.atEnd() && re.isValid()) {
        QString line = reader.readLine();
        QStringList lrcInfo = line.split("]");
        songLrc.append(lrcInfo[1]);
        re.indexIn(lrcInfo[0]);

        QStringList capturedText = re.capturedTexts();
        double time = capturedText.at(1).toDouble() * 60 + capturedText.at(2).toDouble()
                + capturedText.at(3).toDouble() / qPow(10, capturedText.at(3).size());
        qDebug() << time;
        songLrcTime.append(time);

    }
    currentLrcLine = 0;
    ui->lrcLabel->setText(songLrc.at(currentLrcLine));
    qDebug() << "加载歌词完成";
}

//随着音乐播放更新歌词label，实现动态歌词
void MusicPlayer::updateLrc(double time)
{
    if (currentLrcLine == songLrcTime.size())
        return;
    int i;
    for (i = 0; i < songLrcTime.size(); i++) {
        if (time < songLrcTime.at(i)) {
            break;
        }
    }
    ui->lrcLabel->setText(songLrc.at(i - 1));
}

//滑动音量条,改变音量
void MusicPlayer::on_volumeSilder_valueChanged(int value)
{
    musicPlayer->setVolume(value);
}

//点击进度条调整歌曲播放进度
void MusicPlayer::setSliderValue(int value)
{
    musicPlayer->setPosition(value);
}
