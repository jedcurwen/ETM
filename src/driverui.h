#ifndef DRIVERUI_H
#define DRIVERUI_H

#include "mosquitto.h"
#include <QWidget>

namespace Ui {
class driverUI;
}

class driverUI : public QWidget
{
    Q_OBJECT

public:
    explicit driverUI(QString driver, QWidget *parent = nullptr);
    QString currentUser;
    ~driverUI();

private slots:
    void on_editDetails_clicked();
    void on_update_avaliability_clicked();
    void on_connect_to_topic_clicked();
    void onMessageReceived(const QString& topic, const QString& message);
    void on_viewDeliveries_clicked();

    void on_sign_out_clicked();

public slots:
    void onAcceptButtonClicked();
    void onRejectButtonClicked();
    void onUpdateButtonClicked();
private:
    Ui::driverUI *ui;
    int notifications = 0;
    mosquitto* mqtt;
    struct mosquitto *mosq;
};

#endif // DRIVERUI_H
