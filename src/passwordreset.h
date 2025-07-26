#ifndef PASSWORDRESET_H
#define PASSWORDRESET_H

#include <QWidget>

namespace Ui {
class passwordreset;
}

class passwordreset : public QWidget
{
    Q_OBJECT

public:
    explicit passwordreset(QWidget *parent = nullptr);
    ~passwordreset();

private slots:
    void on_updatePass_clicked();

private:
    Ui::passwordreset *ui;
};

#endif // PASSWORDRESET_H
