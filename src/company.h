#ifndef COMPANY_H
#define COMPANY_H

#include <QWidget>
#include "mosquitto.h"
#include "mosquitto_plugin.h"
#include "mosquitto_broker.h"
namespace Ui {
class company;
}

class company : public QWidget
{
    Q_OBJECT

public:
    explicit company(QWidget *parent = nullptr);

    ~company();

private slots:
    void on_view_orders_clicked();

    void onMessageReceived(const QString& topic, const QString& message);



    void on_view_orders_2_clicked();

    void on_sign_out_clicked();

public slots:
    void updateStatusLabel(const QString& text);
    void incrementNotifications();
    void onAcceptButtonClicked();
    void onRejectButtonClicked();
signals:

private:
    Ui::company *ui;
    mosquitto* mqtt;
    struct mosquitto *mosq;
    std::vector<std::string>messages;
    friend void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
    int notifications = 0;

};

#endif // COMPANY_H
