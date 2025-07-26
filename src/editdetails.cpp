#include "editdetails.h"
#include "ui_editdetails.h"
#include<iostream>
#include<QtSql>
#include<QMessageBox>
using namespace std;
EditDetails::EditDetails(QString loggedinUser,QWidget *parent) :
    QWidget(parent),
    myVariable(loggedinUser),
    ui(new Ui::EditDetails)
{
    QLabel *myLabel = new QLabel(this);
    myLabel->setText("Welcome "+myVariable);
    ui->setupUi(this);
    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
        /**
         * @brief username
         * Fields are automatically filled when the window is loaded. This takes the "Logged In User" and gets their username
         * This then executes a simple SQL SELECT statement to fill the corresponding fields.
         */
        QString username = myVariable;

        qDebug() << "Logged In User: " << username;


        // Try to select the user's details from the Customer table
        query.prepare("SELECT Name, Email, Mobile FROM Customer WHERE Username = ?");
        query.bindValue(0, username);
        if(query.exec() && query.next()) {
            QString name = query.value(0).toString();
            QString email = query.value(1).toString();
            QString mobile = query.value(2).toString();

            // Setting the textboxes to values held in the DB
            ui->name->setText(name);
            ui->username->setText(username);
            ui->email->setText(email);
            ui->mobile->setText(mobile);
        } else {
            // Try to select the user's details from the Driver table
            query.prepare("SELECT Name, Email, Mobile FROM Driver WHERE Username = ?");
            query.bindValue(0, username);
            if(query.exec() && query.next()) {
                QString name = query.value(0).toString();
                QString email = query.value(1).toString();
                QString mobile = query.value(2).toString();

                // Setting the textboxes to values held in the DB
                ui->name->setText(name);
                ui->username->setText(username);
                ui->email->setText(email);
                ui->mobile->setText(mobile);
            } else {
                // Try to select the user's details from the Company table
                query.prepare("SELECT Name, Email, Mobile FROM Company WHERE Username = ?");
                query.bindValue(0, username);
                if(query.exec() && query.next()) {
                    QString name = query.value(0).toString();
                    QString email = query.value(1).toString();
                    QString mobile = query.value(2).toString();

                    // Setting the textboxes to values held in the DB
                    ui->name->setText(name);
                    ui->username->setText(username);
                    ui->email->setText(email);
                    ui->mobile->setText(mobile);
                } else {
                    qDebug() << "Error: username not found in any account type";
                }
            }
        }
    }
    EditDetails::~EditDetails()
{
    delete ui;
}
/**
 * @brief EditDetails::on_subBtn_pressed
 * A SQL UPDATE statement is executed to update the fields which have been changed and then inform the User if they have been successful.
 */
void EditDetails::on_subBtn_pressed()
{
    QString name = ui->name->text();
    QString username = ui->username->text();
    QString email = ui->email->text();
    QString mobile = ui->mobile->text();

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    bool success = false;

    // Try to update the user's details in the Customer table
    query.prepare("UPDATE Customer SET Name = ?, Email = ?, Mobile = ? WHERE Username = ?");
    query.addBindValue(name);
    query.addBindValue(email);
    query.addBindValue(mobile);
    query.addBindValue(username);
    if (query.exec() && query.next()) {
        success = true;
    } else {
        query.prepare("UPDATE Driver SET name = ?, Email = ?, Mobile = ? WHERE Username = ?");
        query.addBindValue(name);
        query.addBindValue(email);
        query.addBindValue(mobile);
        query.addBindValue(username);
        if(query.exec()&&query.next()){
             success = true;
        }else{
             query.prepare("UPDATE Company SET companyName = ?, Email = ?, Mobile = ? WHERE Username = ?");
             query.addBindValue(name);
             query.addBindValue(email);
             query.addBindValue(mobile);
             query.addBindValue(username);
             if(query.exec()&&query.next()){
                success = true;
             }else{
               qDebug() << "Error updating company: " << query.lastError().text();
             }
        }
    }

    // If update to Customer table failed, try to update the user's details in the Driver table
    if (!success) {

        if (query.exec()) {
            success = true;
        } else {
            qDebug() << "Error updating driver: " << query.lastError().text();
        }
    }

    // If update to Customer and Driver tables failed, try to update the user's details in the Company table
    if (!success) {


        if (query.exec()) {
            success = true;
        } else {

        }
    }

    // Display message based on whether the update was successful or not
    if (success) {
        qDebug() << "User details updated successfully";
        QMessageBox::information(this, "Success", "User details updated successfully");
    } else {
        qDebug() << "Error updating user details";
        QMessageBox::critical(this, "Error", "Error updating user details");
    }
}


