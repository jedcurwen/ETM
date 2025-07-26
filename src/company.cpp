#include "company.h"
#include <mosquitto.h>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qthread.h"
#include "ui_company.h"
#include "mosquitto_plugin.h"
#include "mosquitto_broker.h"
#include "thread.h"
#include<QtSql>
#include <QWidget>
#include <QPushButton>
#include <QPdfWriter>
#include <QPainter>
#include <QHBoxLayout>
company::company(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::company)
{
    ui->setupUi(this);
    notifications = 0;

    mosquitto_lib_init();

    // Create a new mosquitto client instance
    mqtt = mosquitto_new(NULL, true, this);

    // Connect to the MQTT broker
    int rc = mosquitto_connect(mqtt, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
    }

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));

    query.exec("SELECT COUNT(*) FROM Feedback;");

    if (query.first()){
        ui->noOfReviews->setText(query.value(0).toString());
    }

    query.exec("SELECT AVG(Rating) FROM Feedback;");

    if (query.first()){
        ui->avgRating->setText(query.value(0).toString() + ".0 STARS");
    }
}

company::~company()
{
    delete ui;
}
void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
    qDebug() << "Subscribed to topic successfully!";
}

/**
@brief company::on_view_orders_clicked
*This function is called when the "View Orders" button is clicked.
*It retrieves a list of all orders from the database and displays them in a table.
*/

void company::on_view_orders_clicked() {


    class thread *mqttThread = new class thread(mqtt, "order/new", this);

    // Connect the messageReceived signal of the thread to a slot in your main window
    connect(mqttThread, &thread::messageReceived, this, &company::onMessageReceived);

    // Start the thread
    mqttThread->start();

}
/**
 * @brief company::onMessageReceived
 * @param topic
 * @param message
 * When a new message is received on the topic this function is triggered
 */
void company::onMessageReceived(const QString& topic, const QString& message) // add this method definition
{
    qDebug() << "Message received on topic " << topic << ": " << message<<"\n";
    // Add code to update UI with received message
    messages.push_back(message.toStdString());
    notifications++;
    ui->notificationCount->setText(QString::number(notifications));


    QStringList fields;
    QRegularExpression re("\\b(\\w+)\\s*:\\s*(\\S+)");
    QRegularExpressionMatchIterator matchIter = re.globalMatch(message);
    while (matchIter.hasNext()) {
    QRegularExpressionMatch match = matchIter.next();
    QString key = match.captured(1);
    QString value = match.captured(2);
    fields << key + ":" + value;
    //qDebug() << "Key: " << key << ", Value: " << value;
    }
    qDebug()<<"Message recieved from Customer New Order: "<<message;
    qDebug()<<"Fields: "<<fields;


    QStringList headers = { "OrderID","Postcode From", "Postcode To", "Distance(KM)", "Height(M)", "Length(M)", "Width(M)", "Weight(KG)", "Lorry Type", "Shipping Cost", "Accept", "Reject" };
    ui->viewNewOrdersTable->setColumnCount(headers.size());
    ui->viewNewOrdersTable->setHorizontalHeaderLabels(headers);

    // Add a new row to the table
    int row = ui->viewNewOrdersTable->rowCount();
    ui->viewNewOrdersTable->insertRow(row);

    // Loop over the fields and set the corresponding values in the new row
    for (int i = 0; i < fields.size(); i++) {
    QString field = fields.at(i);
    QString value = field.split(":").at(1).trimmed();
    QTableWidgetItem* item = new QTableWidgetItem(value);
    ui->viewNewOrdersTable->setItem(row, i, item);
    }

    // Add accept and reject buttons to the last two columns
    QPushButton* acceptButton = new QPushButton("Accept");
    QPushButton* rejectButton = new QPushButton("Reject");

    // Set object name for the buttons
    acceptButton->setObjectName(QString("AcceptButton_%1").arg(row));
    rejectButton->setObjectName(QString("RejectButton_%1").arg(row));

    // Connect the clicked signal of the buttons to a slot that will handle the action
    connect(acceptButton, &QPushButton::clicked, this, &company::onAcceptButtonClicked);
    connect(rejectButton, &QPushButton::clicked, this, &company::onRejectButtonClicked);

    ui->viewNewOrdersTable->setCellWidget(row, headers.size()-2, acceptButton);
    ui->viewNewOrdersTable->setCellWidget(row, headers.size()-1, rejectButton);

    // Resize the columns to fit the contents
    ui->viewNewOrdersTable->resizeColumnsToContents();
}
/**
 * @brief company::onAcceptButtonClicked
 * When the "Accept Order" button is activated, this is triggered
 * Orders are sent back to the Cargo Owner to inform them, whilst also publishing messages to
 * driver/order -- For drivers to Accept/Reject
 * company/accepted -- For cargo owners to inform them the order has been accepted into the system.
 * An Invoice is then generated
 */
void company::onAcceptButtonClicked()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    QString time_str = std::ctime(&time);


    QString status = "Accepted by Company";
    QString deliveryStatus = "Further updates will follow";
    // Get the row number from the object name
    QString objectName = sender()->objectName();
    QStringList parts = objectName.split("_");
    int row = parts[1].toInt();

    // Get the OrderID value from the first column of the selected row
    QTableWidgetItem* item = ui->viewNewOrdersTable->item(row, 0);
    QString orderID = item->text();
    QString postcodeTo = ui->viewNewOrdersTable->item(row, 2)->text();
    QString distance = ui->viewNewOrdersTable->item(row, 3)->text();
    QString length = ui->viewNewOrdersTable->item(row, 5)->text();
    QString width = ui->viewNewOrdersTable->item(row, 6)->text();
    QString weight = ui->viewNewOrdersTable->item(row, 7)->text();
    QString shippingCost = ui->viewNewOrdersTable->item(row, 9)->text();



    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    query.prepare("UPDATE Orders SET orderStatus = 'Accepted' WHERE OrderID = ?");
    query.bindValue(0,orderID);

    if(query.exec()){
    qDebug()<<"Order Status Updated";
    }
    else{
    qDebug()<<"Error: "<<query.lastError().text();
    }

    QString payload =
        "Order: " + orderID +
        " To: " + postcodeTo +
        " Distance: " + distance + " km" +
        " Length: " + length + " m" +
        " Width: " + width + " m" +
        " Weight: " + weight + " kg";

    QString customerMessage = "OrderID: "+orderID+
                              "To: "+postcodeTo+
                              "Order Status:"+status+
                              "Delivery Status:"+deliveryStatus;


    // Publish the payload on the "driver/order" topic
    int rc = mosquitto_publish(mqtt, NULL, "driver/order", payload.length(), payload.toUtf8().constData(), 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
    qDebug() << "Failed to publish message: " << mosquitto_strerror(rc);
    }
    qDebug()<<"Message sent to Driver Order";

    rc = mosquitto_publish(mqtt, NULL, "company/accepted", customerMessage.length(), customerMessage.toUtf8().constData(), 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
    qDebug() << "Failed to publish message: " << mosquitto_strerror(rc);
    }
    qDebug()<<"Message sent to Company Accepted";

    ui->viewNewOrdersTable->removeRow(row);
    notifications--;
    ui->notificationCount->setText(QString::number(notifications));

    QString invoice = "Invoice\n\n";
    invoice += "Order Date: " + time_str;
    invoice += "Order ID: " + orderID + "\n";
    invoice += "Delivery Postcode: " + postcodeTo + "\n";
    invoice += "Parcel Length: " + length +"m"+ "\n";
    invoice += "Parcel Width: " + width + "m"+ "\n";
    invoice += "Parcel Weight: " + weight +"kg"+"\n";
    invoice += "Shipping Cost: "+shippingCost + "\n";

    QString fileName = QCoreApplication::applicationDirPath() + "/Invoice_" + orderID + ".pdf";
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize::A4);

    QPainter painter(&writer);
    painter.setFont(QFont("Arial", 12));

    // Split the invoice string by the newline character "\n"
    QStringList invoiceLines = invoice.split("\n");

    // Calculate the height of each line using QFontMetrics
    QFontMetrics fontMetrics(painter.font());
    int lineHeight = fontMetrics.lineSpacing();

    // Draw each line separately
    int y = 100;
    for (int i = 0; i < invoiceLines.size(); i++) {
    painter.drawText(100, y, invoiceLines[i]);
    y += lineHeight; // Increase the y-coordinate by the height of the current line
    }
    painter.end();

    QString invoiceFilePath = QDir::toNativeSeparators(fileName); // Convert the file path to native format

    query.prepare("UPDATE Orders SET Invoice = :invoiceFilePath WHERE OrderID = :orderId");
    query.bindValue(":invoiceFilePath", invoiceFilePath);
    query.bindValue(":orderId", orderID);
    if (!query.exec()) {
    qDebug() << "Failed to update invoice file path for Order ID:" << orderID;
    return;
    }

}
/**
 * @brief company::onRejectButtonClicked
 * This updates the cargo owner that the company has rejected their order. The database is updated.
 */
void company::onRejectButtonClicked()
{
    // Get the row number from the object name
    QString objectName = sender()->objectName();
    QStringList parts = objectName.split("_");
    int row = parts[1].toInt();

    // Get the OrderID value from the first column of the selected row
    QTableWidgetItem* item = ui->viewNewOrdersTable->item(row, 0);
    QString orderID = item->text();

    qDebug() << "Accept Button Pressed for OrderID: " << orderID;
    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    query.prepare("UPDATE Orders SET orderStatus = 'Rejected' WHERE OrderID = ?");
    query.bindValue(0,orderID);

    if(query.exec()){
    qDebug()<<"Order Status Updated";
    }
    else{
    qDebug()<<"Error: "<<query.lastError().text();
    }



    ui->viewNewOrdersTable->removeRow(row);
    notifications--;
    ui->notificationCount->setText(QString::number(notifications));


}

void company::updateStatusLabel(const QString& text)
{
}
void company::incrementNotifications()
{
    notifications++;
    ui->notificationCount->setText(QString::number(notifications));
}

/**
 * @brief company::on_view_orders_2_clicked
 * This displays all Orders stored for all Customers.
 * This gives the Transportation Company valuable information of the amount of orders have been placed.
 */


QString calculateCommission(QString shippingCost){

    double cost = shippingCost.toDouble();

    double commission = cost / 10;

    return QString::number(commission);

}

void company::on_view_orders_2_clicked()
{
    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try {
        query.prepare("SELECT * FROM Orders");
        if(query.exec()) {
            int row = 0;

            ui->orders_table->setRowCount(0);

            while (query.next()) {

                QString username = query.value(0).toString();
                QString orderId = query.value(1).toString();
                QString destPostcode = query.value(3).toString();
                QString distance = query.value(4).toString();
                QString height = query.value(5).toString();
                QString length = query.value(6).toString();
                QString width = query.value(7).toString();
                QString weight = query.value(8).toString();
                QString lorry = query.value(9).toString();
                QString shippingCost = query.value(10).toString();
                QString commission = calculateCommission(shippingCost);
                QString deliveryStatus = query.value(12).toString();
                QString driverName = query.value(13).toString();

                //qDebug()<<username << orderId << destPostcode << distance << height << length << width << weight <<lorry<< shippingCost << deliveryStatus<<driverName;

                ui->orders_table->insertRow(row);

                ui->orders_table->setItem(row, 0, new QTableWidgetItem(username));
                ui->orders_table->setItem(row, 1, new QTableWidgetItem(orderId));
                ui->orders_table->setItem(row, 2, new QTableWidgetItem(destPostcode));
                ui->orders_table->setItem(row, 3, new QTableWidgetItem(distance));
                ui->orders_table->setItem(row, 4, new QTableWidgetItem(height));
                ui->orders_table->setItem(row, 5, new QTableWidgetItem(length));
                ui->orders_table->setItem(row, 6, new QTableWidgetItem(width));
                ui->orders_table->setItem(row, 7, new QTableWidgetItem(weight));
                ui->orders_table->setItem(row, 8, new QTableWidgetItem(lorry));
                ui->orders_table->setItem(row, 9, new QTableWidgetItem(shippingCost));
                ui->orders_table->setItem(row, 10, new QTableWidgetItem(commission));
                ui->orders_table->setItem(row, 11, new QTableWidgetItem(driverName));
                ui->orders_table->setItem(row, 12, new QTableWidgetItem(deliveryStatus));


        }
    }
    }catch (const std::exception& e) {
        qDebug()<<e.what();
}

}

void company::on_sign_out_clicked()
{
this->hide();
}

