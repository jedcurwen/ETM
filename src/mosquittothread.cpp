
#include "mosquittothread.h"
#include "qdebug.h"

MosquittoThread::MosquittoThread(struct mosquitto* mosq) : mosq_(mosq)
{}
    void MosquittoThread::run() {
        int rc = mosquitto_loop_forever(mosq_, -1, 1);
        if (rc != MOSQ_ERR_SUCCESS) {
            qDebug() << "Failed to start Mosquitto loop: " << mosquitto_strerror(rc);
        }
        emit finished();
    }



void MosquittoThread::on_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    // Extract the message payload as a QString
    QString msg = QString::fromUtf8((const char*)message->payload, message->payloadlen);

    // Emit the messageReceived signal
    emit ((MosquittoThread*)userdata)->messageReceived(msg);

}

