#include "createorder.h"
#include "QtSql/qsqldatabase.h"
#include "ui_createorder.h"
#include <iostream>
#include <vector>
#include <QtSql>
#include <sys/types.h>
#include "mosquitto.h"


createOrder::createOrder(QString loggedinUser, QWidget *parent) :
    QWidget(parent),

    ui(new Ui::createOrder),

    currentUsername(loggedinUser)  //retrieve currently logged in customers username
{
    ui->setupUi(this);

    mosquitto_lib_init();

    // Create a new mosquitto client instance
    mqtt = mosquitto_new(NULL, true, this);

    // Set the message callback function


    // Connect to the MQTT broker
    int rc = mosquitto_connect(mqtt, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
    }
}

createOrder::~createOrder()
{
    delete ui;
}
/**
 * @brief calculateCostMultiplier
 * @param weight
 * @param distance
 * @param lorryType
 * @return
 * This determines how much an order will cost based on weight,distance and the lorry size to be assigned
 * This requires inputs such as Weight,Distance to be accurate
 */
int calculateCostMultiplier(double weight, double distance, QString lorryType ){

    int costMultiplier = 1;   //value which will increase based on weight

    if (weight < 100){
        costMultiplier *= 1;
    }
    else if(weight < 500){
        costMultiplier *= 2;
    }
    else{
        costMultiplier *= 3;
    }

    if (lorryType == "Large"){
        costMultiplier *= 3;
    }

    costMultiplier *= distance * 0.1;

    return round(costMultiplier);
}
/**
 * @brief calculateShippingCost
 * @param cargoItem
 * @return
 * This returns the final shipping cost after getting all of the required information
 */
QString calculateShippingCost(cargoDetails cargoItem){

    double volume = cargoItem.height * cargoItem.length * cargoItem.width;

    int costMultiplier = calculateCostMultiplier(cargoItem.weight, cargoItem.distance, cargoItem.lorryType);

    double cost = volume * costMultiplier;

    QString shippingCost = QString::number(cost);

    return shippingCost;
}
/**
 * @brief createOrder::getDetails
 * @return
 * This gets the order details from the cargo owner
 *
 */
cargoDetails createOrder::getDetails() {

    cargoDetails item;

    //retrieving all text fields from window

    item.postcode = ui->postcodeField->displayText();
    item.destination = ui->destinationField->displayText();

    QString inputDistance = ui->distanceField->displayText();
    item.distance = inputDistance.toDouble();

    QString inputHeight = ui->heightField->displayText();
    item.height = inputHeight.toDouble();

    QString inputLength = ui->lengthField->displayText();
    item.length = inputLength.toDouble();

    QString inputWidth = ui->widthField->displayText();
    item.width = inputWidth.toDouble();

    QString inputWeight = ui->weightField->displayText();
    item.weight = inputWeight.toDouble();

    if (item.height > 5 || item.length > 5 || item.width > 5){
        item.lorryType = "Large";
    }
    else{
        item.lorryType = "Small";
    }

    item.shippingCost = calculateShippingCost(item);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString orderID = currentDateTime.toString("yyyyMMddhhmmss") + QString::number(rand());

    // Assign the unique identifier to the item
    item.orderID = orderID;

    return item;

}
/**
 * @brief createOrder::on_calculateShippingBtn_clicked
 *
 * The overall cost of shipping is displayed to the Cargo Owner
 */
void createOrder::on_calculateShippingBtn_clicked()
{
    cargoDetails item = getDetails();

    QString costLabelText = "Shipping cost = £";
    QString shippingCostString = costLabelText + item.shippingCost;
    ui->shippingCostLabel->setText(shippingCostString);

    QString lorryLabelText = "Lorry type = ";
    QString lorryTypeString = lorryLabelText + item.lorryType;
    ui->lorryTypeLabel->setText(lorryTypeString);
}
/**
 * @brief createOrder::on_requestOrderBtn_clicked
 * The request order is sent to the transportation company via a message broker on order/new where the Transportation company
 * can either Accept/Reject the order.
 * This then starts a thread to store the connection to the broker whilst being able to maintain application functionality
 */
void createOrder::on_requestOrderBtn_clicked(){

    cargoDetails item = getDetails();

    QSqlQuery query(QSqlDatabase::database("ETM_DB"));

    query.exec("INSERT INTO Orders(Username,OrderID,userPostcode,destinationPostcode,distanceInKm,"
               "heightInM,lengthInM,widthInM,weightInKg,lorryType,shippingCost,orderStatus,deliveryStatus"
               ") VALUES ('"+currentUsername+"','"+item.orderID+"','"+item.postcode+"','"+item.destination+"',"+QString::number(item.distance)+
               ","+QString::number(item.height)+","+QString::number(item.length)+","+QString::number(item.width)+
               ","+QString::number(item.weight)+",'"+item.lorryType+
               "','"+item.shippingCost+"','Not accepted','Not delivered');");

    QString message =
                        "Order: " + item.orderID
                      + " From: " + item.postcode
                      + " To: " + item.destination
                      + " Distance: " + QString::number(item.distance) + " km"
                      + " Height: " + QString::number(item.height) + " m"
                      + " Length: " + QString::number(item.length) + " m"
                      + " Width: " + QString::number(item.width) + " m"
                      + " Weight: " + QString::number(item.weight) + " kg"
                      + " Lorry Type: " + item.lorryType
                      + " Shipping Cost: " + item.shippingCost;

    qDebug()<<message;
    try{
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    int rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to connect to broker: " << mosquitto_strerror(rc);
        mosquitto_destroy(mosq);
        throw std::runtime_error("Failed to connect to broker");
    }
    qDebug() << "connected to broker";

    // publish the message to the specified topic

    rc = mosquitto_publish(mosq, NULL, "order/new", strlen(message.toUtf8().constData()), message.toUtf8().constData(), 0, false);

    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to publish message: " << mosquitto_strerror(rc);
    }

    // start the network loop in a separate thread
    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        qDebug() << "Failed to start network loop: " << mosquitto_strerror(rc);
    }

    // wait for a moment to let the message be sent
    QThread::msleep(100);

    mosquitto_disconnect(mosq);

    // stop the network loop thread
    mosquitto_loop_stop(mosq, true);

    // destroy the mosq object
    mosquitto_destroy(mosq);
    }catch(const std::exception& e){
    qDebug() << "Exception caught: " << e.what();
    }

    this->hide();
}

