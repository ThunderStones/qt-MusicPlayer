#include "smallwidget.h"
#include "ui_smallwidget.h"


SmallWidget::SmallWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SmallWidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->albumPic->setScaledContents(true);
}

SmallWidget::~SmallWidget()
{
    delete ui;
}

void SmallWidget::setPic(QString s)
{
    ui->albumPic->setPixmap(QPixmap(QString(s)));
}

void SmallWidget::setSongName(QString s)
{
    ui->songName->setText(s);
}
