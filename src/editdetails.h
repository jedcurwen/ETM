#ifndef EDITDETAILS_H
#define EDITDETAILS_H

#include <QWidget>
#include <QLabel>
namespace Ui {
class EditDetails;
}

class EditDetails : public QWidget
{
    Q_OBJECT

public:
    explicit EditDetails(QString loggedinUser,QWidget *parent = nullptr);
    ~EditDetails();
    QString myVariable;
private slots:
    void on_subBtn_pressed();

private:
    Ui::EditDetails *ui;
    QLabel *welcomeLabel;
    QString loggedinUser;

};

#endif // EDITDETAILS_H
