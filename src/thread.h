
#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include "mosquitto.h"


class thread : public QThread
{
    Q_OBJECT
public:
    explicit thread(mosquitto* mqtt, const QString& topic, QObject* parent = nullptr);


signals:
    void messageReceived(const QString& topic,const QString& message);

protected:
    void run()override;

private:
    mosquitto *mqtt_;
    static void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
    static void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
    QString topic_;

};

#endif // THREAD_H
