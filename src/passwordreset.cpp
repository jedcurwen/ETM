#include "passwordreset.h"
#include "ui_passwordreset.h"
#include<iostream>
#include<QtSql>
#include<QMessageBox>
using namespace std;
passwordreset::passwordreset(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::passwordreset)
{
    ui->setupUi(this);
    ui->passerror->setVisible(false);
}

passwordreset::~passwordreset()
{
    delete ui;
}

/**
 * @brief passwordreset::on_updatePass_clicked
 * Changes password based on the new input details.
 */

void passwordreset::on_updatePass_clicked()
{
    QSqlDatabase db;
    db=QSqlDatabase::addDatabase("QSQLITE","ETM_DB");
    db.setDatabaseName("ETM_DB.sqlite");
    if(!db.open()){
        cout<<"Error Database not found.";
    }else
    {
        QString username = ui->usernameTxt->text();
        QSqlQuery query(QSqlDatabase::database("ETM_DB"));
        query.prepare("SELECT COUNT(*) FROM Customer WHERE Username=:username");
        query.bindValue(":username",username);
        query.exec();
        if(query.first()){
            int rows = query.value(0).toInt();
            if (rows==0){
            QMessageBox::critical(this, tr("Error"),tr("User Not In Database"));  //if select returns no rows then user must not be in database
            }else{
                QString password1 = ui->passwordTxt->text();
                QString password2 = ui->passConf->text();
                if(password1==password2){
                    std::size_t passwordHash = std::hash<std::string>{}(password1.toStdString());
                    QString pswdHash = QString::number(passwordHash);
                    query.prepare("UPDATE Customer SET PasswordHash = ? WHERE Username = ? ");
                    query.addBindValue(pswdHash);
                    query.addBindValue(username);
                    if(!query.exec()){
                        qDebug() << "Error updating user:" << query.lastError().text();
                    }else{
                        qDebug() << "Updated successfully";

                        //setting text fields to null
                        ui->username->setText("");
                        ui->username->setText("");
                        ui->username->setText("");
                    }
                }
                else{
                    cout<<password1.toStdString()<<password2.toStdString()<<endl;
                    ui->passerror->setText("Password dont match.");
                    ui->passerror->setVisible(true);

                }
            }
       }
    }
}

