#ifndef LEAVEFEEDBACK_H
#define LEAVEFEEDBACK_H

#include <QWidget>

namespace Ui {
class leaveFeedback;
}

class leaveFeedback : public QWidget
{
    Q_OBJECT

public:
    explicit leaveFeedback(QWidget *parent = nullptr);
    ~leaveFeedback();

private slots:

    void on_submitButton_clicked();

private:
    Ui::leaveFeedback *ui;
};

#endif // LEAVEFEEDBACK_H
