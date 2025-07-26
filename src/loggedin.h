#ifndef LOGGEDIN_H
#define LOGGEDIN_H

#include <QWidget>
#include <QLabel>
#include "createorder.h"


namespace Ui {
class LoggedIn;
}

class LoggedIn : public QWidget
{
    Q_OBJECT

public:
    explicit LoggedIn(QString loggedinUser, QWidget *parent = nullptr);
    ~LoggedIn();
    QString username;

    static void on_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);

public slots:
    void on_viewEditDetailsBtn_clicked();
    void onMessageReceived(const QString& message);
    void onViewInvoiceClicked();


private slots:

    void on_signOutBtn_pressed();
    void on_createOrderBtn_clicked();
    void on_updateOrderTableBtn_clicked();
    void on_connect_clicked();



    void on_clear_order_updates_clicked();

    void on_feedbackButton_clicked();

signals:
    void userLoggedIn(QString username);

private:
    Ui::LoggedIn *ui;
    QLabel *welcomeLabel;
    QString loggedinUser;
    mosquitto* mqtt;
    struct mosquitto *mosq;
    int notifications = 0;
};

#endif // LOGGEDIN_H
