#include "createuser.h"
#include "ui_createuser.h"
#include<iostream>
#include<QtSql>
#include<QMessageBox>
#include <QFileDialog>
#include <functional>

using namespace std;
createUser::createUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::createUser)
{
    ui->setupUi(this);

    ui->customerGroupBox->setEnabled(true);
    ui->driverGroupBox->setEnabled(false);
    ui->companyGroupBox->setEnabled(false);
}

createUser::~createUser()
{
    delete ui;
}
/**
 * @brief createUser::on_customerRadio_pressed
 * Users select their Account Type based on their role
 */
void createUser::on_customerRadio_pressed()
{
    ui->customerGroupBox->setEnabled(true);
    ui->driverGroupBox->setEnabled(false);
    ui->companyGroupBox->setEnabled(false);
}
/**
 * @brief createUser::on_driverRadio_pressed
 * Users select their Account Type based on their role
 */
void createUser::on_driverRadio_pressed()
{
    ui->customerGroupBox->setEnabled(false);
    ui->driverGroupBox->setEnabled(true);
    ui->companyGroupBox->setEnabled(false);
}
/**
 * @brief createUser::on_companyRadio_pressed
 * Users select their Account Type based on their role
 */
void createUser::on_companyRadio_pressed()
{
    ui->customerGroupBox->setEnabled(false);
    ui->driverGroupBox->setEnabled(false);
    ui->companyGroupBox->setEnabled(true);
}
/**
 * @brief createUser::on_uploadPhoto_clicked
 * This allows Drivers to upload a photo of themselves (Only required for Drivers)
 * By opening up a FileDialog users can browse for files, but only image extensions are accepted such as - .png,.jpg,.bmp
 */
void createUser::on_uploadPhoto_clicked()
{
    imagePath = QFileDialog::getOpenFileName(this, tr("Choose Profile Picture"), "", tr("Image Files (*.png *.jpg *.bmp)"));

    /*
        if (!imagePath.isEmpty()) {
            QImage image(imagePath);
            ui->photoLabel->setPixmap(QPixmap::fromImage(image));
        }

    */
}
/**
@brief createUser::createCustomerAccount
*This function creates a new customer account by checking if the user is not already a duplicate (based on their username).
*If the user is not a duplicate, a password hash is generated to securely store the user's password in the database to prevent account compromise.
*The user's information is then inserted into the database and a confirmation message is displayed to the user.
*If the user is a driver or a company, additional fields such as driver license number or company name are also collected and stored in the database.
*/
void createUser::createCustomerAccount(){

    QString name = ui->customerName->text();
    QString username = ui->customerUsername->text();
    QString password = ui->customerPassword->text();
    QString email = ui->customerEmail->text();
    QString address = ui->customerAddress->text();
    QString mobile = ui->customerMobile->text();

    size_t passwordHash = std::hash<std::string>{}(password.toStdString());

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try{
    query.exec("SELECT COUNT(*) FROM Customer WHERE Username = '" +username+ "';");   //Checking if entered username is already in the table
    if(query.first()){

        int rows = query.value(0).toInt();
        if (rows>0){
        QMessageBox::critical(this, tr("Error"),tr("User Exists"));

        }else{
        QString mobile = ui->customerMobile->text();
        QRegularExpression phoneRegex("^[0-9]*$");
        QRegularExpressionMatch match = phoneRegex.match(mobile);
        if(!match.hasMatch()) {
            QMessageBox::critical(this, tr("Error"),tr("Mobile number should only contain digits."));
            return;
        }

            QString pswdHash = QString::number(passwordHash);
            query.exec("INSERT INTO Customer(Name,Username,PasswordHash,Email,Address,Mobile)"
                        "VALUES ('"+name+"','"+username+"','"+pswdHash+"','"+email+"','"+address+"','"+mobile+"');");
            QMessageBox::critical(this, tr("Save"),tr("Saved"));
        }

        }
        }        catch(const std::exception& e){
                QMessageBox::critical(this, tr("Error"), tr("Failed to save data: ") + QString::fromStdString(e.what()));
    }
}

void createUser::createCompanyAccount()
{

    QString name = ui->companyName->text();
    QString username = ui->companyUsername->text();
    QString password = ui->companyPassword->text();
    QString email = ui->companyEmail->text();
    QString address = ui->companyAddress->text();
    QString mobile = ui->companyPhone->text();

    size_t passwordHash = std::hash<std::string>{}(password.toStdString());

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try{
    query.exec("SELECT COUNT(*) FROM Company WHERE Username = '" +username+ "';");   //Checking if entered username is already in the table
    if(query.first())
    {

        int rows = query.value(0).toInt();
        if (rows>0)
        {
        QMessageBox::critical(this, tr("Error"),tr("User Exists"));
        }
        else
        {
        QRegularExpression phoneRegex("^[0-9]*$");
        QRegularExpressionMatch match = phoneRegex.match(mobile);
        if(!match.hasMatch())
        {
            QMessageBox::critical(this, tr("Error"),tr("Mobile number should only contain digits."));
            return;
        }

            QString pswdHash = QString::number(passwordHash);
            query.exec("INSERT INTO Company(companyName,Username,PasswordHash,Email,Address,Mobile)"
                        "VALUES ('"+name+"','"+username+"','"+pswdHash+"','"+email+"','"+address+"','"+mobile+"');");
            QMessageBox::critical(this, tr("Save"),tr("Saved"));
        }
    }
        }catch(const std::exception& e){
        QMessageBox::critical(this, tr("Error"), tr("Failed to save data: ") + QString::fromStdString(e.what()));
    }
}

void createUser::createDriverAccount()
{

    QString name = ui->driverName->text();
    QString username = ui->driverUsername->text();
    QString password = ui->driverPassword->text();
    QString email = ui->driverEmail->text();
    QString address = ui->driverAddress->text();
    QString mobile = ui->driverMobile->text();
    QString niNumber = ui->niNumber->text();
    QString licenceNo = ui->licenceNo->text();
    QString cpcNumber = ui->cpcNo->text();
    QString lorryReg = ui->lorryRegNumber->text();
    QString lorryWeight = ui->lorryWeight->text();
    QString lorryType = ui->lorryTypeCombo->currentText();

    size_t passwordHash = std::hash<std::string>{}(password.toStdString());

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try
    {
    query.exec("SELECT COUNT(*) FROM Driver WHERE Username = '" +username+ "';");   //Checking if entered username is already in the table
    if(query.first())
    {

        int rows = query.value(0).toInt();
        if (rows>0)
        {
        QMessageBox::critical(this, tr("Error"),tr("User Exists"));
        }
        else{
        QRegularExpression phoneRegex("^[0-9]*$");
        QRegularExpressionMatch match = phoneRegex.match(mobile);
        if(!match.hasMatch())
        {
            QMessageBox::critical(this, tr("Error"),tr("Mobile number should only contain digits."));
            return;
        }

            QString pswdHash = QString::number(passwordHash);
            query.exec("INSERT INTO Driver(name,Username,PasswordHash,Email,Address,Mobile,niNumber,licenceNumber,"
                       "cpcNumber,lorryRegNumber,lorryWeight,lorryType)"
                        "VALUES ('"+name+"','"+username+"','"+pswdHash+"','"+email+"','"+address+"','"+mobile+
                       "','"+niNumber+"','"+licenceNo+"','"+cpcNumber+"','"+lorryReg+"','"+lorryWeight+"','"+lorryType+"'"
                       ");");
            QMessageBox::critical(this, tr("Save"),tr("Saved"));
        }

        }
        }        catch(const std::exception& e){
                 QMessageBox::critical(this, tr("Error"), tr("Failed to save data: ") + QString::fromStdString(e.what()));
    }
}

void createUser::on_subBtn_clicked()
{

    string role;

    if(ui->customerRadio->isChecked()){
        role = "Customer";
        createCustomerAccount();

    }
    else if(ui->driverRadio->isChecked()){
        role = "Driver";
        createDriverAccount();

    }else{
        role="Company";
        createCompanyAccount();
    }
}

void createUser::on_loginBtn_clicked()
{
    this -> hide();
}



