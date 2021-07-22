#include "registerdialog.h"
#include "ui_registerdialog.h"

registerDialog::registerDialog(QSqlDatabase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::registerDialog)
{
    ui->setupUi(this);
    this->db = db;
    this->setWindowTitle("注册");
    createTable();
}

registerDialog::~registerDialog()
{
    delete ui;
}

//建立用户数据库
void registerDialog::createTable()
{
    if (!db.isOpen())
        db.open("admin", "psw");

    QSqlQuery query;
    bool success = query.exec("create table userInfo"
                              "(nickName varchar,"
                              "userName varchar primary key,"
                              "password varchar,"
                              "likeMusicIndex varchar)");
    if (success) {
        qDebug() << "userInfo table created successfully";
    } else {
        qDebug() << "fail to create userInfo";
    }
}


void registerDialog::on_regBtn_clicked()
{

    QString nickName = ui->nickName->text();
    QString userName = ui->userName->text();
    QString password = ui->password->text();

    if (nickName.isEmpty() || userName.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, "提示", "请填写所有信息");
        qDebug() << "info not enough";
        return;
    }

    QSqlQuery query;
    query.exec(QString("select userName from userInfo where userName = \"%1\"").arg(userName));
    query.next();
    if (query.value(0).toString() == userName) {
        QMessageBox::information(this, QString("注册失败"), QString("用户名重复，请更换用户名。"));
        return;
    }


    query.prepare("insert into userInfo values(?,?,?,?)");
    query.bindValue(0, nickName);
    query.bindValue(1, userName);
    query.bindValue(2, password);
    query.bindValue(3, "");

    bool success = query.exec();
    if (success) {
        QMessageBox::information(this, QString("注册成功"), QString("注册成功，请前往登录。"));
        qDebug() << "register successfully";
        this->close();
    } else {
        QMessageBox::information(this, QString("注册失败"), QString("发生未知错误。"));
        qDebug() << query.lastError().driverText();
    }
}
