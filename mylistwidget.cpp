#include "mylistwidget.h"
#include <QMouseEvent>
#include "main.cpp"

/*
 * myListWidget类继承QListWidget类
 * 创建这个类是为了重写右键菜单事件
 */

myListWidget::myListWidget(QWidget * parents):QListWidget (parents)
{
    play = new QAction("播放", this);
    like = new QAction("添加到喜欢", this);
    unlike = new QAction("不喜欢", this);
    download = new QAction("下载", this);
    deleteSong = new QAction("从列表中删除", this);

    popMenu = new QMenu(this);

    connect(play, &QAction::triggered, this, &myListWidget::emitPlay);
    connect(like, &QAction::triggered, this, &myListWidget::emitLike);
    connect(unlike, &QAction::triggered, this, &myListWidget::emitUnlike);
    connect(download, &QAction::triggered, this, &myListWidget::emitDownload);
    connect(deleteSong, &QAction::triggered, this, &myListWidget::emitDelete);
}

//重写父类的contextMenuEvent函数，绘制右键菜单
//由于右键菜单的内容需要根据具体的列表来显示内容，将this指针通过信号发送到musicwidget类中处理
void myListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (this->itemAt(event->pos()) == nullptr)
        return;

    emit listRightClicked(this);

}

/*
 * 以下函数为具体action被点击时，发送信号到musicwidget处理
 */
void myListWidget::emitPlay()
{
    qDebug() << "play";
    emit transferSignals(this, actPlay);
}

void myListWidget::emitLike()
{
    qDebug() << "like";
    emit transferSignals(this, actLike);
}

void myListWidget::emitUnlike()
{
    emit transferSignals(this, actUnlike);
}

void myListWidget::emitDownload()
{
    qDebug() << "download";
    emit transferSignals(this, actDownload);
}

void myListWidget::emitDelete()
{
    emit transferSignals(this, actDelete);
}
