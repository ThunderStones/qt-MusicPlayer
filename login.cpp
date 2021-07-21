#include "login.h"
#include "ui_login.h"
#include "registerdialog.h"
login::login(QSqlDatabase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    this->db = db;
    this->setWindowTitle("登录");

}

login::~login()
{
    delete ui;
}

//按下登录按钮
void login::on_registerBtn_clicked()
{
    registerDialog regDialog(db, this);
    regDialog.exec();
}

//按下登录按钮
void login::on_loginBtn_clicked()
{
    QString userName = ui->userNameText->text();
    QString password = ui->passwordText->text();
    if (userName.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, "提示", "用户名或密码不能为空。");
        return;
    }

    QSqlQuery query;
    query.exec(QString("select userName,password from userInfo where userName = \"%1\" and password = \"%2\"").arg(userName, password));
    query.next();
    if (query.isValid()) {
        QMessageBox::information(this, "成功", "登录成功");
        emit userLogin(userName);
        this->close();
    }
    else {
        QMessageBox::information(this, "错误", "用户名或密码错误");
        return;
    }
}
