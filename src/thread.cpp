#include "thread.h"
#include <QDebug>

thread::thread(mosquitto *mqtt, const QString& topic, QObject *parent) : QThread(parent), mqtt_(mqtt), topic_(topic)
{
    if(!mqtt_){
        qDebug() << "Error creating Mosquitto instance";
        return;
    }

    int rc = mosquitto_connect(mqtt_, "localhost", 1883, 60);
    if(rc != MOSQ_ERR_SUCCESS){
        qDebug() << "Error connecting to broker: " << mosquitto_strerror(rc);
        return;
    }

    mosquitto_subscribe_callback_set(mqtt_, [](mosquitto*, void*, int, int, const int*){});
    mosquitto_message_callback_set(mqtt_, [](mosquitto*, void* userdata, const mosquitto_message* message){
        if (message->payloadlen > 0) {
            QString topic = QString::fromUtf8(message->topic);
            QString payload = QString::fromUtf8(static_cast<const char*>(message->payload), message->payloadlen);

            thread* th = static_cast<thread*>(userdata);

            // Only emit the messageReceived signal if the received message topic matches the subscribed topic
            if (topic == th->topic_) {
                emit th->messageReceived(topic, payload);
            }
        }
    });

    int result = mosquitto_loop_start(mqtt_);
    if (result != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to start Mosquitto loop: " << mosquitto_strerror(result);
        return;
    }

    rc = mosquitto_subscribe(mqtt_, NULL, qPrintable(topic_), 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to subscribe to topic " << topic_ << ": " << mosquitto_strerror(rc);
        return;
    }
}

void thread::run()
{

    mosquitto_lib_init();

    int rc = mosquitto_connect(mqtt_, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {

        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
        return;
    }

    rc = mosquitto_subscribe(mqtt_, NULL, "order/new/#", 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to subscribe to topic: " << mosquitto_strerror(rc);

        return;
    }
    rc = mosquitto_subscribe(mqtt_, NULL, "driver/order/#", 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to subscribe to topic: " << mosquitto_strerror(rc);
        return;
    }

    mosquitto_subscribe_callback_set(mqtt_, on_subscribe);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to subscribe to topic: " << mosquitto_strerror(rc);
        return;
    }

    mosquitto_message_callback_set(mqtt_, on_message);

    mosquitto_user_data_set(mqtt_, this); // Pass a pointer to the thread object to the library

    rc = mosquitto_loop_forever(mqtt_, -1, 1);
    if (rc != MOSQ_ERR_SUCCESS) {

        qDebug() << "Failed to start Mosquitto loop: " << mosquitto_strerror(rc);
        return;
    }


}
void thread::on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    thread* instance = static_cast<thread*>(userdata);
    QString topic = QString::fromUtf8(message->topic);
    QString payload = QString::fromUtf8((char*)message->payload, message->payloadlen);

    // Filter messages based on the topic
    if (topic == instance->topic_) {
        emit instance->messageReceived(topic, payload);
    }
}

void thread::on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{

    qDebug() << "Subscribed to topic successfully!";

}

