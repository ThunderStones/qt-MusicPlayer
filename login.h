#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QtSql>
namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    login();
    explicit login(QSqlDatabase &db, QWidget *parent = nullptr);
    ~login();

private slots:
    void on_registerBtn_clicked();

    void on_loginBtn_clicked();

private:
    Ui::login *ui;
    QSqlDatabase db;

signals:
    void userLogin(QString userName);

};

#endif // LOGIN_H
