#ifndef ADMINLOGIN_H
#define ADMINLOGIN_H

#include <QMainWindow>
#include <conserial.h>

namespace Ui {
class Adminlogin;
}

class Adminlogin : public QMainWindow
{
    Q_OBJECT

public:
    explicit Adminlogin(QWidget *parent = nullptr);
    ~Adminlogin();

private slots:
    void on_Login_clicked();


signals:
    void firstWindow();

private:
    Ui::Adminlogin *ui;
    hwe::Conserial stand_;
};

#endif // ADMINLOGIN_H
