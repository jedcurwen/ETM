#include "loggedin.h"
#include "createorder.h"
#include "editdetails.h"
#include "leavefeedback.h"
#include "ui_loggedin.h"
#include<iostream>
#include<QtSql>
#include "loggedin.h"
#include"mosquitto.h"
#include "mosquittothread.h"
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include<QString>
#include <QDesktopServices>
using namespace std;
LoggedIn::LoggedIn(QString loggedinUser,QWidget *parent) : QWidget(parent),

    /**
     *When the LoggedIn window loads, we initially create a connection to the Broker when we can send/receive messages
     *We also set the "Logged In User" to the current Username of the person using the application
     */
    username(loggedinUser),
    ui(new Ui::LoggedIn)
{
    int notifications =0;
    QLabel *myLabel = new QLabel(this);
    myLabel->setText("Welcome "+username);

    ui->setupUi(this);

    //Connect to Broker
    mosquitto_lib_init();

    // Create a new mosquitto client instance
    mqtt = mosquitto_new(NULL, true, this);

    // Connect to the MQTT broker
    int rc = mosquitto_connect(mqtt, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
    }
}

LoggedIn::~LoggedIn()
{
    delete ui;
}
/**
 * @brief LoggedIn::on_viewEditDetailsBtn_clicked
 * Users can edit their details, the current name of the logged in user is passed to fill in the fields within this window.
 */
void LoggedIn::on_viewEditDetailsBtn_clicked()
{
    EditDetails *editDetails = new EditDetails(username);
    editDetails->show();

}
/**
 * @brief LoggedIn::on_signOutBtn_pressed
 * This ends the session and logs out the User.
 */
void LoggedIn::on_signOutBtn_pressed()
{
    this->hide();
}

/**
 * @brief LoggedIn::on_createOrderBtn_clicked
 * Cargo Owners are able to create new orders, this opens up another window called CreateOrder
 */
void LoggedIn::on_createOrderBtn_clicked()
{
    createOrder *createOrderWindow = new createOrder(username,this);
    createOrderWindow->show();

}


/**
 * @brief LoggedIn::on_updateOrderTableBtn_clicked
 * This is a function to display all of the Orders which belong to the User.
 * Surrounded by Try-Catch statements we will be able to catch any exceptions should the expected statement fail.
 */
void LoggedIn::on_updateOrderTableBtn_clicked()
{


    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try{
    query.exec("SELECT COUNT(*) from Orders WHERE Username = '" + username + "';");

    if (query.first()){
        ui->orderListTable->setRowCount(query.value(0).toInt());
    }

    query.exec("SELECT * from Orders WHERE Username = '" + username + "';");

    for (int i=0; query.next(); i++){


        QString orderID = query.value(1).toString();
        ui->orderListTable->setItem( i, 0, new QTableWidgetItem(orderID));

        QString destination = query.value(3).toString();
        ui->orderListTable->setItem( i, 1, new QTableWidgetItem(destination));

        QString orderStatus = query.value(11).toString();
        ui->orderListTable->setItem( i, 2, new QTableWidgetItem(orderStatus));

        QString deliveryStatus = query.value(12).toString();
        ui->orderListTable->setItem( i, 3, new QTableWidgetItem(deliveryStatus));

        QTableWidgetItem *item = new QTableWidgetItem();
        QPushButton *button = new QPushButton("View Invoice");
        ui->orderListTable->setCellWidget(i, 4, button);
        button->setProperty("orderId",orderID);
        connect(button, &QPushButton::clicked, this, &LoggedIn::onViewInvoiceClicked);
    }


}catch(const std::exception& e){
    qDebug()<<e.what();

}
}
/**
 * @brief LoggedIn::on_connect_clicked
 * This connects us to the topic on which we will be receiving our messages (company/accepted)
 * This is also starts a new thread called "MosquittoThread" to allow us to maintain application functionality and have no deadlocks when the connection is active
 * Messages received are then placed in a table in real-time when orders have been accepted by the Transportation Company.
 */
void LoggedIn::on_connect_clicked()
{
    mosq = mosquitto_new(NULL, true, this);
    mosquitto_message_callback_set(mosq, &on_message_callback);
    int rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
        mosquitto_destroy(mosq);
        return;
    }

    rc = mosquitto_subscribe(mosq, NULL, "company/accepted", 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to subscribe to topic: " << mosquitto_strerror(rc);
        mosquitto_destroy(mosq);
        return;
    }

    // Start the Mosquitto loop in a new thread
    QThread* thread = new QThread;
    MosquittoThread* mosquittoThread = new MosquittoThread(mosq);
    mosquittoThread->moveToThread(thread);

    connect(thread, &QThread::started, mosquittoThread, &MosquittoThread::run);
    connect(mosquittoThread, &MosquittoThread::messageReceived, this, &LoggedIn::onMessageReceived);
    connect(mosquittoThread, &MosquittoThread::finished, thread, &QThread::quit);

    thread->start();

    qDebug() << "Connected to broker and subscribed to company/accepted topic";
}


void LoggedIn::on_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    // Cast the userdata pointer to a LoggedIn* pointer
    LoggedIn *loggedIn = static_cast<LoggedIn*>(userdata);

    // Call the onMessageReceived() method on the LoggedIn instance
    loggedIn->onMessageReceived(QString::fromUtf8(static_cast<const char*>(message->payload)));

}

void LoggedIn::onMessageReceived(const QString& message)
{
    qDebug() << "Message received: " << message;
    notifications++;
    ui->notification_count->setText(QString::number(notifications));


    QStringList fields;
    QRegularExpression re("\\b(\\w+)\\s*:\\s*(\\S+)");
    QRegularExpressionMatchIterator matchIter = re.globalMatch(message);
    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QString key = match.captured(1);
        QString value = match.captured(2);
        fields << key + ":" + value;
        qDebug() << "Key: " << key << ", Value: " << value;
    }

    qDebug()<<"Fields: "<<fields;

    QStringList headers = { "OrderID","Destination", "Order Status", "Delivery Status"};
    ui->updates_table->setColumnCount(headers.size());
    ui->updates_table->setHorizontalHeaderLabels(headers);

    QHeaderView* header = ui->updates_table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    // Add a new row to the table
    int row = ui->updates_table->rowCount();
    ui->updates_table->insertRow(row);

    // Loop over the fields and set the corresponding values in the new row
    for (int i = 0; i < fields.size(); i++) {
        QString field = fields.at(i);
        QString value = field.split(":").at(1).trimmed();
        QTableWidgetItem* item = new QTableWidgetItem(value);
        ui->updates_table->setItem(row, i, item);
    }
    // Resize the columns to fit the contents
    ui->updates_table->resizeColumnsToContents();
}
/**
 * @brief LoggedIn::on_clear_order_updates_clicked
 * This will allow the user to clear the contents of the table where accepted orders are placed to help maintain a user-friendly
 * and easily viewable experience.
 */
void LoggedIn::on_clear_order_updates_clicked()
{
     ui->updates_table->clearContents();
     ui->updates_table->setRowCount(0);
     notifications=0;
     ui->notification_count->setText(QString::number(notifications));


}
void LoggedIn::onViewInvoiceClicked() {
     QPushButton *button = qobject_cast<QPushButton *>(sender());
     QString orderId = button->property("orderId").toString();

     // Get the invoice filepath from the database
     QSqlQuery query(QSqlDatabase::database("ETM_DB"));
     query.prepare("SELECT Invoice FROM Orders WHERE OrderID = :orderId");
     query.bindValue(":orderId", orderId);
     if (!query.exec() || !query.first()) {
        qDebug() << "Failed to retrieve invoice filepath for Order ID:" << orderId;
        return;
     }
     QString invoiceFilePath = query.value(0).toString();

     // Open the invoice file in the default PDF viewer
     QDesktopServices::openUrl(QUrl::fromLocalFile(invoiceFilePath));
}


void LoggedIn::on_feedbackButton_clicked()
{

    leaveFeedback *newFeedback = new leaveFeedback();
    newFeedback->show();

}

