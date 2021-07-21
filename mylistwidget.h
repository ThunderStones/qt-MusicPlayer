#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H
#include <QListWidget>
#include <QAction>
#include <QMenu>
#include <QDebug>

class myListWidget : public QListWidget
{
    Q_OBJECT

public:
    myListWidget(QWidget * parent = nullptr);

    QMenu * popMenu;

    QAction * play;
    QAction * like;
    QAction * unlike;
    QAction * download;
    QAction * deleteSong;

    enum actionType {actPlay, actLike, actUnlike, actDownload, actDelete};
protected:
    void contextMenuEvent(QContextMenuEvent *event);
private:



private slots:
    void emitPlay();
    void emitLike();
    void emitUnlike();
    void emitDownload();
    void emitDelete();

signals:
    void transferSignals(myListWidget *, actionType);
    void listRightClicked(myListWidget *);
};

#endif // MYLISTWIDGET_H
