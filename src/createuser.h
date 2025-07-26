#ifndef CREATEUSER_H
#define CREATEUSER_H

#include <QWidget>

namespace Ui {
class createUser;
}

class createUser : public QWidget
{
    Q_OBJECT

public:
    explicit createUser(QWidget *parent = nullptr);

    ~createUser();

private slots:
    void on_customerRadio_pressed();

    void on_driverRadio_pressed();

    void on_subBtn_clicked();

    void on_uploadPhoto_clicked();

    void createCustomerAccount();

    void createDriverAccount();

    void createCompanyAccount();

    void on_loginBtn_clicked();

    void on_companyRadio_pressed();



private:

    QString imagePath;
    Ui::createUser *ui;


};

#endif // CREATEUSER_H
