#ifndef CREATEORDER_H
#define CREATEORDER_H

#include "mosquitto.h"
#include <QWidget>

namespace Ui {
class createOrder;
}

struct cargoDetails{
    QString postcode;
    QString destination;
    double distance;
    double height;
    double length;
    double width;
    double weight;
    QString lorryType;
    QString shippingCost;
    QString orderNumber;
    QString orderID;
};

class createOrder : public QWidget
{
    Q_OBJECT

public:
    explicit createOrder(QString loggedinUser, QWidget *parent = nullptr);
    ~createOrder();
    Ui::createOrder *ui;
    QString postcode, destination, height, length, weight, currentUsername;

private slots:
    void on_calculateShippingBtn_clicked();
    void on_requestOrderBtn_clicked();
    cargoDetails getDetails();

private:
    mosquitto* mqtt;
    struct mosquitto *mosq;
};



#endif // CREATEORDER_H
