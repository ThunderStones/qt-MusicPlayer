#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
namespace Ui {
class registerDialog;
}

class registerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit registerDialog(QSqlDatabase &db, QWidget *parent = nullptr);
    ~registerDialog();

private slots:
    void on_regBtn_clicked();

private:
    Ui::registerDialog *ui;
    QSqlDatabase db;

    void createTable();
};

#endif // REGISTERDIALOG_H
