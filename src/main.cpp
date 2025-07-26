#include "mainwindow.h"
#include<iostream>
#include<QtSql>
#include<QMessageBox>
#include <QApplication>
#include <vector>
using namespace std;

/**
 * @brief createTables
 * Here we will place the SQL commands for creating the tables that we require for the main application.
 */
void createTables(){

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));

    query.exec("CREATE TABLE IF NOT EXISTS Customer("
            "Name VARCHAR,"
            "Username VARCHAR PRIMARY KEY,"
            "PasswordHash VARCHAR,"
            "Email VARCHAR,"
            "Address VARCHAR,"
            "Mobile INT"
            ");");

    query.exec("CREATE TABLE IF NOT EXISTS Driver("
            "name VARCHAR,"
            "Username VARCHAR PRIMARY KEY,"
            "PasswordHash VARCHAR,"
            "Email VARCHAR,"
            "Address VARCHAR,"
            "Mobile INT,"
            "niNumber INT,"
            "licenceNumber INT,"
            "cpcNumber INT,"
            "lorryRegNumber INT,"
            "lorryWeight REAL,"
            "lorryType VARCHAR"
            ");");

    query.exec("CREATE TABLE IF NOT EXISTS Company("
            "companyName VARCHAR,"
            "Username VARCHAR PRIMARY KEY,"
            "PasswordHash VARCHAR,"
            "Email VARCHAR,"
            "Address VARCHAR,"
            "Mobile INT"
            ");");

    query.exec("CREATE TABLE IF NOT EXISTS Orders("
            "Username VARCHAR,"
            "OrderID VARCHAR,"
            "userPostcode VARCHAR,"
            "destinationPostcode VARCHAR,"
            "distanceInKm REAL,"
            "heightInM REAL,"
            "lengthInM REAL,"
            "widthInM REAL,"
            "weightInKg REAL,"
            "lorryType VARCHAR,"
            "shippingCost VARCHAR,"
            "orderStatus VARCHAR,"
            "deliveryStatus VARCHAR,"
            "deliveryDriver VARCHAR,"
            "Invoice BLOB,"
            "FOREIGN KEY(Username) REFERENCES Customer(Username)"
            ");");

    query.exec("CREATE TABLE IF NOT EXISTS DriverOrders("
               "DriverName VARCHAR,"
               "Username VARCHAR,"
               "OrderID VARCHAR,"
               "destinationPostcode VARCHAR,"
               "distanceInKm REAL,"
               "heightInM REAL,"
               "lengthInM REAL,"
               "widthInM REAL,"
               "weightInKg REAL,"
               "shippingCost INT,"
               "orderStatus VARCHAR,"
               "deliveryStatus VARCHAR,"
               "FOREIGN KEY(Username) REFERENCES Orders(Username)"
               ");");

    query.exec("CREATE TABLE IF NOT EXISTS Feedback("
               "CompanyName VARCHAR,"
               "Rating INT,"
               "FOREIGN KEY(CompanyName) REFERENCES Company(companyName)"
               ");");
}

int main(int argc, char *argv[])
{

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","ETM_DB");
    db.setDatabaseName("ETM_DB.sqlite");

    createTables();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec(); 
}
