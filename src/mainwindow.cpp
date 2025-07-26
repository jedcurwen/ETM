#include "mainwindow.h"
#include "company.h"
#include "driverui.h"
#include "passwordreset.h"
#include "ui_mainwindow.h"
#include "createuser.h"
#include<iostream>
#include<QtSql>
#include<QMessageBox>
#include"loggedin.h"

using namespace std;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::on_createUserBtn_clicked
 * Create new user window
 */
void MainWindow::on_createUserBtn_clicked()
{
    createUser *newUser = new createUser();
    //this->hide();
    newUser->show();
}

/**
 * @brief MainWindow::invalidCredentials
 * What happens when the credentials are invalid.
 * Text in text fields are removed so user can enter different details
 */

void MainWindow::invalidCredentials(){

    QMessageBox::critical(this, tr("Error"),tr("Username/Password Invalid."));
    ui->userName->setText("");
    ui->passWord->setText("");
}

/**
 * @brief MainWindow::openUserWindow
 * @param userRole
 * @param username
 * Determines and opens correct type type of window to open based on the type of user.
 */

void MainWindow::openUserWindow(QString userRole,QString username){

    if(userRole == "Customer"){
        LoggedIn *loggedInWindow = new LoggedIn(username, this);
        loggedInWindow->show();

    }else if (userRole == "Driver"){
        driverUI *driver = new driverUI(username,nullptr);
        driver->show();
    }
    else{
        company *companyWindow = new company(this);
        companyWindow->show();
    }
}

/**
 * @brief MainWindow::on_loginBtn_clicked
 * Checks login details to find out whether they are in the database.
 */

void MainWindow::on_loginBtn_clicked(){

    QString username = ui->userName->text();
    QString password = ui->passWord->text();
    size_t passwordHash = std::hash<std::string>{}(password.toStdString());

    QString userRole = ui->roleCombo->currentText();
    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    query.exec("SELECT PasswordHash from "+userRole+" WHERE Username='"+username+"';");

    if(query.first()){
        size_t requiredPasswordHash = std::stoull(query.value(0).toString().toStdString(), 0, 10);     //Select password for comparison.

        if(passwordHash==requiredPasswordHash){

            if(query.first()){
                openUserWindow(userRole, username);
            }
        }else{
            invalidCredentials();
        }
    }
}

/**
 * @brief MainWindow::on_forgottenPassword_clicked
 * brings up password reset window
 */

void MainWindow::on_forgottenPassword_clicked()
{
    passwordreset *reset = new passwordreset();
    reset->show();
}

