
#ifndef MOSQUITTOTHREAD_H
#define MOSQUITTOTHREAD_H



#include <QObject>
#include <mosquitto.h>

class MosquittoThread : public QObject
{
    Q_OBJECT

public:
    MosquittoThread(struct mosquitto* mosq);

public slots:
    void run();

signals:
    void messageReceived(const QString& message);
    void finished();

private:
    struct mosquitto* mosq_;

    static void on_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);
};


#endif // MOSQUITTOTHREAD_H
