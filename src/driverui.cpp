#include "driverui.h"
#include "editdetails.h"
#include "qpushbutton.h"
#include "ui_driverui.h"
#include "thread.h"
#include "qthread.h"
#include<QtSql>
#include<QMessageBox>
#include <QHBoxLayout>
driverUI::driverUI(QString driver, QWidget *parent) : QWidget(parent),
    currentUser(driver),
    ui(new Ui::driverUI)
{

    ui->setupUi(this);
    ui->avaliabilityCombo->addItem("Avaliable");
    ui->avaliabilityCombo->addItem("Unavaliable");
    notifications = 0;

    ui->avaliabilityLabel->setText("Avaliable");


    QLabel *myLabel = new QLabel(this);
    myLabel->setText("Welcome "+currentUser);

    mosquitto_lib_init();

    // Create a new mosquitto client instance
    mqtt = mosquitto_new(NULL, true, this);

    // Connect to the MQTT broker
    int rc = mosquitto_connect(mqtt, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);

    }
}

driverUI::~driverUI()
{
    delete ui;
}
/**
 * @brief driverUI::on_editDetails_clicked
 * A new window allowing users to Edit their details
 * The current user is passed to this to allow for automatic field completion of most common field updates
 */
void driverUI::on_editDetails_clicked()
{
    EditDetails *editDetails = new EditDetails(currentUser);
    editDetails->show();

}


void driverUI::on_update_avaliability_clicked()
{
    ui->avaliabilityLabel->setText(ui->avaliabilityCombo->currentText());
}

/**
 * @brief driverUI::on_connect_to_topic_clicked
 * This will connect to the Message Broker to allow for drivers to recieve real-time messages of new orders
 * This will happen on a seperate thread to maintain application usability
 */
void driverUI::on_connect_to_topic_clicked()
{
    class thread *mqttThread = new class thread(mqtt,"driver/order", this);

    // Connect the messageReceived signal of the thread to a slot in your main window
    connect(mqttThread, &thread::messageReceived, this, &driverUI::onMessageReceived);

    // Start the thread
    mqttThread->start();

}
/**
 * @brief driverUI::onMessageReceived
 * @param topic
 * @param message
 * When a new message is received on the topic this function is triggered
 * This will return the TOPIC & MESSAGE what has been received.
 * The results will be displayed, wrapped with Accept/Reject buttons to allow drivers to Accept/Reject orders.
 */
void driverUI::onMessageReceived(const QString& topic, const QString& message) // add this method definition
{
    if(ui->avaliabilityLabel->text()=="Avaliable"){
    qDebug() << "Message received on topic " << topic << ": " << message<<"\n";
    // Add code to update UI with received message

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

    QStringList headers = { "OrderID","Postcode To", "Distance(KM)", "Length(M)", "Width(M)", "Weight(M)", "Accept", "Reject" };
    ui->new_deliveries->setColumnCount(headers.size());
    ui->new_deliveries->setHorizontalHeaderLabels(headers);

    // Add a new row to the table
    int row = ui->new_deliveries->rowCount();
    ui->new_deliveries->insertRow(row);

    // Loop over the fields and set the corresponding values in the new row
    for (int i = 0; i < fields.size(); i++) {
        QString field = fields.at(i);
        QString value = field.split(":").at(1).trimmed();
        QTableWidgetItem* item = new QTableWidgetItem(value);
        ui->new_deliveries->setItem(row, i, item);
    }

    // Add accept and reject buttons to the last two columns
    QPushButton* acceptButton = new QPushButton("Accept");
    QPushButton* rejectButton = new QPushButton("Reject");

    // Set object name for the buttons
    acceptButton->setObjectName(QString("AcceptButton_%1").arg(row));
    rejectButton->setObjectName(QString("RejectButton_%1").arg(row));

    // Connect the clicked signal of the buttons to a slot that will handle the action
    connect(acceptButton, &QPushButton::clicked, this, &driverUI::onAcceptButtonClicked);
    connect(rejectButton, &QPushButton::clicked, this, &driverUI::onRejectButtonClicked);

    ui->new_deliveries->setCellWidget(row, headers.size()-2, acceptButton);
    ui->new_deliveries->setCellWidget(row, headers.size()-1, rejectButton);

    // Resize the columns to fit the contents
    ui->new_deliveries->resizeColumnsToContents();
    }
}
/**
 * @brief driverUI::onAcceptButtonClicked
 * When the "Accept Order" button is activated, this is triggered Orders are updated for the cargo owner in the database to
 * inform them the order has been accepted, this is then assigned to the driver.
 * Shipment updates can then be made by this driver to further keep the cargo owner informed.
 *
 */
void driverUI::onAcceptButtonClicked()
{
    // Get the row number from the object name
    QString objectName = sender()->objectName();
    QStringList parts = objectName.split("_");
    int row = parts[1].toInt();
    try {


    // Get the OrderID value from the first column of the selected row
    QTableWidgetItem *order = ui->new_deliveries->item(row, 0);
    if (order != nullptr) {
        QString text = order->text();
        QString orderID = text.split(":").last().trimmed();
        qDebug() << "Order accepted with ID: " << orderID;
        QString postcodeTo = ui->new_deliveries->item(row, 1)->text();
        QString distance = ui->new_deliveries->item(row, 2)->text();
        QString length = ui->new_deliveries->item(row, 3)->text();
        QString width = ui->new_deliveries->item(row, 4)->text();
        QString weight = ui->new_deliveries->item(row, 5)->text();

        QSqlQuery query(QSqlDatabase::database("ETM_DB"));
        query.prepare("SELECT * FROM Orders WHERE OrderID = :orderID");
        query.bindValue(":orderID", orderID);
        query.exec();

        if (query.next()) {
            QString customerName = query.value(0).toString();
            qDebug() << "Customer name: " << customerName;
            QString orderID = query.value(1).toString();
            QString postcodeTo = query.value(3).toString();
            QString distance = query.value(4).toString();
            QString height = query.value(5).toString();
            QString length = query.value(6).toString();
            QString width = query.value(7).toString();
            QString weight = query.value(8).toString();
            QString shippingCost = query.value(10).toString();
            QString orderStatus = query.value(11).toString();
            QString deliveryStatus = query.value(12).toString();

            qDebug()<<"VALUES FROM SQL QUERY: "<<customerName<<orderID<<postcodeTo<<distance<<height<<length<<width<<weight<<shippingCost<<orderStatus<<deliveryStatus;
            query.prepare("INSERT INTO DriverOrders(DriverName,Username,OrderID,destinationPostcode,distanceInKm,heightInM,lengthInM,widthInM,weightInKg,shippingCost,orderStatus,deliveryStatus ) "
                          "VALUES(:driverName,:customerName,:orderID,:postcodeTo,:distance,:height,:length,:width,:weight,:shippingCost,:orderStatus,'Accepted by Driver')");
            query.bindValue(":driverName", currentUser);
            query.bindValue(":customerName", customerName);
            query.bindValue(":orderID", orderID);
            query.bindValue(":postcodeTo", postcodeTo);
            query.bindValue(":distance", distance);
            query.bindValue(":height", height);
            query.bindValue(":length", length);
            query.bindValue(":width", width);
            query.bindValue(":weight", weight);
            query.bindValue(":shippingCost", shippingCost);
            query.bindValue(":orderStatus", orderStatus);
            query.bindValue(":deliveryStatus", deliveryStatus);

            if(query.exec()){
                qDebug()<<"Data has been updated successfully!";
                query.prepare("UPDATE Orders SET deliveryStatus ='Accepted by Driver', deliveryDriver = :driverName WHERE orderID=:orderID");
                query.bindValue(":driverName",currentUser);
                query.bindValue(":orderID",orderID);

                if(query.exec()){
                    qDebug()<<"Data Updated!";
                }
            }else{
                qDebug()<<"Error updating!";
            }
        } else {
            qDebug() << "No customer found with order ID: " << orderID;
        }

    } else {
        qDebug() << "Item is null!";
    }
   }
    catch(const std::exception& e){
                    QMessageBox::critical(this, tr("Error"), tr("Failed to save data: ") + QString::fromStdString(e.what()));

}
}
/**
 * @brief driverUI::onRejectButtonClicked
 * This will inform the cargo owner that the order has been rejected.
 * A database update will then be executed.
 */
void driverUI::onRejectButtonClicked()
{
    // Get the row number from the object name
    QString objectName = sender()->objectName();
    QStringList parts = objectName.split("_");
    int row = parts[1].toInt();

    // Get the OrderID value from the first column of the selected row
    try {
    // Get the OrderID value from the first column of the selected row
    QTableWidgetItem *order = ui->new_deliveries->item(row, 0);
    if (order != nullptr) {
        QString text = order->text();
        QString orderID = text.split(":").last().trimmed();
        qDebug() << "Order Rejected with ID: " << orderID;

        QSqlQuery query(QSqlDatabase::database("ETM_DB"));
        query.prepare("UPDATE Orders SET deliveryStatus ='Rejected by Driver' WHERE orderID=:orderID");
        query.bindValue(":orderID",orderID);

        if(query.exec()){
            qDebug()<<"Data Updated!";
        }
    }




}catch(const std::exception& e){
        QMessageBox::critical(this, tr("Error"), tr("Failed to save data: ") + QString::fromStdString(e.what()));
    }
}
/**
 * @brief driverUI::on_viewDeliveries_clicked
 * All deliveries for the specified driver will be displayed, an update button will also be included along with a combobox of
 * order updates. These are then passed to the cargo owner.
 */
void driverUI::on_viewDeliveries_clicked()
{
    //Clear existing items in the table
    ui->existing_orders->clearContents();

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try {
        query.prepare("SELECT * FROM DriverOrders WHERE DriverName =:driverName");
        query.bindValue(":driverName",currentUser);
        if(query.exec()) {
            int row = 0;
            while (query.next()) {



                QString username = query.value(1).toString();
                QString orderId = query.value(2).toString();
                QString destPostcode = query.value(3).toString();
                QString distance = query.value(4).toString();
                QString height = query.value(5).toString();
                QString length = query.value(6).toString();
                QString width = query.value(7).toString();
                QString weight = query.value(8).toString();
                QString shippingCost = query.value(9).toString();
                QString deliveryStatus = query.value(11).toString();

                ui->existing_orders->insertRow(row);
                QComboBox* comboBox = new QComboBox();
                comboBox->addItem("Delivered");
                comboBox->addItem("In Transit");
                comboBox->addItem("Pending");
                comboBox->setCurrentText(deliveryStatus);

                QPushButton* updateButton = new QPushButton("Update");

                updateButton->setObjectName(QString("UpdateButton_%1").arg(row));

                connect(updateButton, &QPushButton::clicked, this, &driverUI::onUpdateButtonClicked);

                ui->existing_orders->setItem(row, 0, new QTableWidgetItem(username));
                ui->existing_orders->setItem(row, 1, new QTableWidgetItem(orderId));
                ui->existing_orders->setItem(row, 2, new QTableWidgetItem(destPostcode));
                ui->existing_orders->setItem(row, 3, new QTableWidgetItem(distance));
                ui->existing_orders->setItem(row, 4, new QTableWidgetItem(height));
                ui->existing_orders->setItem(row, 5, new QTableWidgetItem(length));
                ui->existing_orders->setItem(row, 6, new QTableWidgetItem(width));
                ui->existing_orders->setItem(row, 7, new QTableWidgetItem(weight));
                ui->existing_orders->setItem(row, 8, new QTableWidgetItem(shippingCost));
                 ui->existing_orders->setCellWidget(row, 9, comboBox);
                ui->existing_orders->setCellWidget(row, 10, updateButton);
                ui->existing_orders->resizeColumnsToContents();

                row++;
            }
        }
    } catch (const std::exception& e) {
       qDebug()<<e.what();
    }
}
/**
 * @brief driverUI::onUpdateButtonClicked
 * This updates the cargo owner's shipment with the value of the delivery status combo box.
 */
void driverUI::onUpdateButtonClicked()
{
    qDebug() << "Update Button Clicked";

    // Get the button that was clicked
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    // Get the row and column of the button
    QModelIndex buttonIndex = ui->existing_orders->indexAt(button->pos());
    int row = buttonIndex.row();
    int col = buttonIndex.column();

    // Get the QComboBox widget in the same row as the button
    QWidget* widget = ui->existing_orders->cellWidget(row, col - 1);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    if (!comboBox) {
        return;
    }

    // Get the selected delivery status from the QComboBox
    QString deliveryStatus = comboBox->currentText();
    qDebug() << "Selected delivery status:" << deliveryStatus;

    // Get the customer name from the first column in the same row as the button
    QTableWidgetItem* item = ui->existing_orders->item(row, 0);
    QString customerName = item ? item->text() : "";
    qDebug() << "Customer name:" << customerName;

    QTableWidgetItem* orderID = ui->existing_orders->item(row, 1);
    QString order = orderID ? orderID->text() : "";
    qDebug() << "Customer name:" << order;

    // Update the order with the selected delivery status
    QSqlQuery query(QSqlDatabase::database("ETM_DB"));
    try {

        query.prepare("UPDATE Orders SET deliveryStatus = :deliveryStatus");
        query.bindValue(":deliveryStatus",deliveryStatus);
        if(query.exec()){
            qDebug()<<"Order"<<order<<" Has been updated";
        }
        else{
            qDebug()<<"Couldn't update!";
        }

    } catch (const std::exception& e) {
        qDebug()<<e.what();
    }
}

void driverUI::on_sign_out_clicked()
{
    this->hide();
}

