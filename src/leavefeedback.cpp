#include "leavefeedback.h"
#include "ui_leavefeedback.h"
#include <iostream>
#include<QtSql>

leaveFeedback::leaveFeedback(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::leaveFeedback)
{
    ui->setupUi(this);

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));

    query.exec("SELECT companyName from Company;");

    while(query.next()){

        QString company = query.value(0).toString();

        ui->companyCombo->addItem(company);

    }

}

leaveFeedback::~leaveFeedback()
{
    delete ui;
}

void leaveFeedback::on_submitButton_clicked()
{

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));

    QString company = ui->companyCombo->currentText();

    QString rating = ui->ratingCombo->currentText();

    query.exec("INSERT INTO Feedback(CompanyName,Rating) VALUES ('" + company + "','" + rating + "');");

    this->close();
}

